//
// Created by dantezhu on 14-10-14.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

#include <winsock2.h>
#pragma comment(lib,"pthreadVSE2.lib")
#define SLEEP Sleep;

#else
#include <unistd.h>
#include <arpa/inet.h>
#define SLEEP sleep;
#endif

#include "cocos2d.h"

#include "NetService.h"
#include "Delegate.h"

// 等待下次连接时间
#define CONNECT_SLEEP_TIME 1

// 队列的大小
#define MSG_FROM_SERVER_SIZE 100
#define MSG_TO_SERVER_SIZE 100

// LOG TAG
#define LOG_TAG   "ferry"

namespace ferry {

    template<class BoxType>
    NetService<BoxType>::NetService() {
        m_enabled = false;
        pthread_mutex_init(&m_enabled_mutex, NULL);
        pthread_cond_init(&m_enabled_cond, NULL);

        m_port = 0;
        m_msgQueueFromServer = new BlockQueue<Message *>(MSG_FROM_SERVER_SIZE);
        m_msgQueueToServer = new BlockQueue<BoxType *>(MSG_TO_SERVER_SIZE);
        m_client = nullptr;

        m_should_connect = false;

        m_sendBox = nullptr;
        m_recvMsg = nullptr;

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
	netkit::Stream::startupSocket();
#endif

    }

    template<class BoxType>
    NetService<BoxType>::~NetService() {
        pthread_mutex_destroy(&m_enabled_mutex);
        pthread_cond_destroy(&m_enabled_cond);

        if (m_client) {
            delete m_client;
            m_client = nullptr;
        }

        if (m_msgQueueFromServer) {
            delete m_msgQueueFromServer;
            m_msgQueueFromServer = nullptr;
        }

        if (m_msgQueueToServer) {
            delete m_msgQueueToServer;
            m_msgQueueToServer = nullptr;
        }

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
	netkit::Stream::cleanupSocket();
#endif

    }

    template<class BoxType>
    bool NetService<BoxType>::init(Delegate<BoxType> *delegate, std::string host, short port) {
        m_delegate = delegate;
        m_host = host;
        m_port = port;
        return true;
    }

    template<class BoxType>
    void NetService<BoxType>::start() {
        if (m_enabled) {
            // 已经调用过一次，是不能再调用的
            return;
        }
        _setEnabled(true);

        // 标记要连接服务器
        connect();
        // 启动线程
        _startThreads();
        // 注册到主线程执行
        _registerMainThreadSchedule();
    }

    template<class BoxType>
    void NetService<BoxType>::stop() {
        _setEnabled(false);
    }

    template<class BoxType>
    bool NetService<BoxType>::isConnected() {
        return m_client == nullptr ? true : !m_client->isClosed();
    }

    template<class BoxType>
    int NetService<BoxType>::connect() {
        m_should_connect = true;
        return 0;
    }

    template<class BoxType>
    inline void NetService<BoxType>::_setEnabled(bool enabled) {
        pthread_mutex_lock(&m_enabled_mutex);
        m_enabled = enabled;
        if (m_enabled) {
            // 只通知可用
            pthread_cond_broadcast(&m_enabled_cond);
        }
        pthread_mutex_unlock(&m_enabled_mutex);
    }

    template<class BoxType>
    void NetService<BoxType>::_connectToServer() {
        _closeConn();
        // 没有超时
        m_client = new netkit::TcpClient(m_host, m_port, -1);

        int ret = m_client->connectServer();
        if (ret) {
            // 链接失败了
            // 没关系，下个循环还会继续重连
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
        else {
            // 分发链接成功的消息
            _onConnOpen();
        }
    }

    template<class BoxType>
    void* NetService<BoxType>::_recvMsgFromServerThreadWorker(void *args) {
        NetService<BoxType>* netService = (NetService<BoxType>*)args;
        netService->_recvMsgFromServer();

        return nullptr;
    }

    template<class BoxType>
    void* NetService<BoxType>::_sendMsgToServerThreadWorker(void *args) {
        NetService<BoxType>* netService = (NetService<BoxType>*)args;
        netService->_sendMsgToServer();

        return nullptr;
    }

    template<class BoxType>
    void NetService<BoxType>::_closeConn() {
        if (m_client) {
            m_client->closeStream();
            delete m_client;
            m_client = nullptr;
        }
    }

    template<class BoxType>
    void NetService<BoxType>::_startThreads() {
        _startRecvMsgFromServerThread();
        _startSendMsgToServerThread();
    }

    template<class BoxType>
    void NetService<BoxType>::_startRecvMsgFromServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &NetService<BoxType>::_recvMsgFromServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    template<class BoxType>
    void NetService<BoxType>::_startSendMsgToServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &NetService<BoxType>::_sendMsgToServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    template<class BoxType>
    void NetService<BoxType>::_recvMsgFromServer() {
        int ret;

        while (1) {
            if (!m_enabled) {
                pthread_mutex_lock(&m_enabled_mutex);
                // 锁定后再确认一下
                if (!m_enabled) {
                    // 只通知可用
                    pthread_cond_wait(&m_enabled_cond, &m_enabled_mutex);
                }
                pthread_mutex_unlock(&m_enabled_mutex);
            }

            if (!isConnected()) {
                if (m_should_connect) {
                    // 连接服务器
                    _connectToServer();
                }

                if (!isConnected()) {
                    // 如果还没有建立链接，就等一下
                    SLEEP(CONNECT_SLEEP_TIME);
                }
                continue;
            }

            BoxType* box = new BoxType();

            ret = m_client->read(box);
            if (ret < 0) {
                // 统一按照断掉链接处理
                _onConnClose();
            }
            else {
                // 有数据
                _onMessageFromServer(box);
            }
        }
    }

    template<class BoxType>
    void NetService<BoxType>::_sendMsgToServer() {
        int ret;

        while (1) {
            if (!m_enabled) {
                pthread_mutex_lock(&m_enabled_mutex);
                // 锁定后再确认一下
                if (!m_enabled) {
                    // 只通知可用
                    pthread_cond_wait(&m_enabled_cond, &m_enabled_mutex);
                }
                pthread_mutex_unlock(&m_enabled_mutex);
            }
            ret = m_msgQueueToServer->pop(m_sendBox);

            if (ret == 0) {
                if (m_client && m_sendBox) {
                    m_client->write(m_sendBox);
                    delete m_sendBox;
                    m_sendBox = nullptr;
                }
            }
        }
    }

    template<class BoxType>
    void NetService<BoxType>::_onConnOpen() {
        Message * msg = new Message();
        msg->what = DELEGATE_MSG_OPEN;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void NetService<BoxType>::_onConnClose() {
        // 设置为不重连，等触发connectToServer再改状态
        m_should_connect = false;

        Message * msg = new Message();
        msg->what = DELEGATE_MSG_CLOSE;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void NetService<BoxType>::_onMessageFromServer(BoxType* box) {
        Message * msg = new Message();
        msg->what = DELEGATE_MSG_RECV;
        msg->box = box;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void NetService<BoxType>::_onMainThreadReceiveMessage(Message *msg) {
        if (!msg) {
            cocos2d::log("[%s]-[%s][%d][%s] null msg", LOG_TAG, __FILE__, __LINE__, __FUNCTION__);
            return;
        }
        if (!m_delegate) {
            cocos2d::log("[%s]-[%s][%d][%s] null delegate", LOG_TAG, __FILE__, __LINE__, __FUNCTION__);
            return;
        }

        switch (msg->what) {
            case DELEGATE_MSG_OPEN:
                m_delegate->onOpen(this);
                break;
            case DELEGATE_MSG_RECV:
                m_delegate->onMessage(this, msg->box);
                break;
            case DELEGATE_MSG_CLOSE:
                m_delegate->onClose(this);
                break;
            default:
                cocos2d::log("[%s]-[%s][%d][%s] msg.what: %d", LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                        msg->what);
        }

        // 不清除box
        delete msg;
    }

    template<class BoxType>
    void NetService<BoxType>::_registerMainThreadSchedule() {
        auto func = [this](float dt){
            // 只要有数据就拼命循环完
            while (1) {
                int ret = this->m_msgQueueFromServer->pop_nowait(m_recvMsg);
                if (ret == 0) {
                    this->_onMainThreadReceiveMessage(m_recvMsg);
                }
                else {
                    break;
                }
            }
        };

        cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, "ferry_ui_loop");
    }
}
