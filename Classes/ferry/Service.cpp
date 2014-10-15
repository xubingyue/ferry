//
// Created by dantezhu on 14-10-14.
//
#ifndef __SERVICE_CPP_20141015151947__
#define __SERVICE_CPP_20141015151947__


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

#include <winsock2.h>
#pragma comment(lib,"pthreadVSE2.lib")
#define FERRY_SLEEP(sec) Sleep((sec)*1000);

#else
#include <unistd.h>
#include <arpa/inet.h>
#define FERRY_SLEEP(sec) sleep(sec);
#endif

#include "cocos2d.h"

#include "Service.h"
#include "Delegate.h"


namespace ferry {

#define FERRY_LOG_TAG   "ferry"

    enum DELEGATE_MSG_TYPE {
        DELEGATE_MSG_OPEN=1,
        DELEGATE_MSG_RECV=2,
        DELEGATE_MSG_CLOSE=3,
    };

// 等待下次连接时间
    const int CONNECT_SLEEP_TIME_SEC = 1;

// 队列的大小
    const int MSG_FROM_SERVER_SIZE = 100;
    const int MSG_TO_SERVER_SIZE = 100;



    template<class BoxType>
    Service<BoxType>::Service() {
        m_enable = false;
        pthread_mutex_init(&m_enable_mutex, NULL);
        pthread_cond_init(&m_enable_cond, NULL);

        m_port = 0;
        m_msgQueueFromServer = new BlockQueue<Message<BoxType> *>(MSG_FROM_SERVER_SIZE);
        m_msgQueueToServer = new BlockQueue<BoxType *>(MSG_TO_SERVER_SIZE);
        m_client = nullptr;

        m_should_connect = false;

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
	netkit::Stream::startupSocket();
#endif

    }

    template<class BoxType>
    Service<BoxType>::~Service() {
        pthread_mutex_destroy(&m_enable_mutex);
        pthread_cond_destroy(&m_enable_cond);

        _clearMsgQueues();

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
    bool Service<BoxType>::init(Delegate<BoxType> *delegate, std::string host, short port) {
        m_delegate = delegate;
        m_host = host;
        m_port = port;
        return true;
    }

    template<class BoxType>
    void Service<BoxType>::start() {
        if (m_enable) {
            // 已经调用过一次，是不能再调用的
            return;
        }
        _setEnable(true);

        // 标记要连接服务器
        connect();
        // 启动线程
        _startThreads();
        // 注册到主线程执行
        _registerMainThreadSchedule();
    }

    template<class BoxType>
    void Service<BoxType>::stop() {
        _setEnable(false);
        // 关闭连接
        _closeConn();
    }

    template<class BoxType>
    bool Service<BoxType>::isConnected() {
        return m_client == nullptr ? false : !m_client->isClosed();
    }

    template<class BoxType>
    bool Service<BoxType>::isRunning() {
        return m_enable;
    }

    template<class BoxType>
    int Service<BoxType>::connect() {
        m_should_connect = true;
        return 0;
    }

    template<class BoxType>
    void Service<BoxType>::send(BoxType *box) {
        cocos2d::log("[%s]-[%s][%d][%s] cmd: %d, sn: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                box->cmd, box->sn);

        m_msgQueueToServer->push_nowait(box);
    }

    template<class BoxType>
    inline void Service<BoxType>::_setEnable(bool enable) {
        pthread_mutex_lock(&m_enable_mutex);
        m_enable = enable;
        if (m_enable) {
            // 只通知可用
            pthread_cond_broadcast(&m_enable_cond);
        }
        pthread_mutex_unlock(&m_enable_mutex);
    }

    template<class BoxType>
    void Service<BoxType>::_connectToServer() {
        _closeConn();
        // 没有超时
        m_client = new netkit::TcpClient(m_host, m_port, -1);

        int ret = m_client->connectServer();
        if (ret) {
            // 链接失败了
            // 没关系，下个循环还会继续重连
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
        else {
            // 分发链接成功的消息
            _onConnOpen();
        }
    }

    template<class BoxType>
    void* Service<BoxType>::_recvMsgFromServerThreadWorker(void *args) {
        Service<BoxType>* netService = (Service<BoxType>*)args;
        netService->_recvMsgFromServer();

        return nullptr;
    }

    template<class BoxType>
    void* Service<BoxType>::_sendMsgToServerThreadWorker(void *args) {
        Service<BoxType>* netService = (Service<BoxType>*)args;
        netService->_sendMsgToServer();

        return nullptr;
    }

    template<class BoxType>
    void Service<BoxType>::_closeConn() {
        // 关闭，就要把所有消息先清空
        _clearMsgQueues();

        if (m_client) {
            m_client->closeStream();
            delete m_client;
            m_client = nullptr;
        }
    }

    template<class BoxType>
    void Service<BoxType>::_startThreads() {
        _startRecvMsgFromServerThread();
        _startSendMsgToServerThread();
    }

    template<class BoxType>
    void Service<BoxType>::_startRecvMsgFromServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &Service<BoxType>::_recvMsgFromServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    template<class BoxType>
    void Service<BoxType>::_startSendMsgToServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &Service<BoxType>::_sendMsgToServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    template<class BoxType>
    void Service<BoxType>::_recvMsgFromServer() {
        int ret;

        while (1) {
            if (!m_enable) {
                pthread_mutex_lock(&m_enable_mutex);
                // 锁定后再确认一下
                if (!m_enable) {
                    // 只通知可用
                    pthread_cond_wait(&m_enable_cond, &m_enable_mutex);
                }
                pthread_mutex_unlock(&m_enable_mutex);
            }

            if (!isConnected()) {
                if (m_should_connect) {
                    // 连接服务器
                    _connectToServer();
                }

                if (!isConnected()) {
                    // 如果还没有建立链接，就等一下
                    FERRY_SLEEP(CONNECT_SLEEP_TIME_SEC);
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
    void Service<BoxType>::_sendMsgToServer() {
        BoxType* box = nullptr;

        int ret;

        while (1) {
            if (!m_enable) {
                pthread_mutex_lock(&m_enable_mutex);
                // 锁定后再确认一下
                if (!m_enable) {
                    // 只通知可用
                    pthread_cond_wait(&m_enable_cond, &m_enable_mutex);
                }
                pthread_mutex_unlock(&m_enable_mutex);
            }
            ret = m_msgQueueToServer->pop(box);

            if (ret == 0) {
                if (m_client && box) {
                    m_client->write(box);
                    delete box;
                    box = nullptr;
                }
            }
        }
    }

    template<class BoxType>
    void Service<BoxType>::_onConnOpen() {
        cocos2d::log("[%s]-[%s][%d][%s]", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__);

        Message<BoxType> * msg = new Message<BoxType>();
        msg->what = DELEGATE_MSG_OPEN;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void Service<BoxType>::_onConnClose() {
        cocos2d::log("[%s]-[%s][%d][%s]", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__);

        // 设置为不重连，等触发connectToServer再改状态
        m_should_connect = false;

        Message<BoxType> * msg = new Message<BoxType>();
        msg->what = DELEGATE_MSG_CLOSE;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void Service<BoxType>::_onMessageFromServer(BoxType* box) {
        cocos2d::log("[%s]-[%s][%d][%s] cmd: %d, sn: %d, ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                box->cmd, box->sn, box->ret);

        Message<BoxType> * msg = new Message<BoxType>();
        msg->what = DELEGATE_MSG_RECV;
        msg->box = box;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] cmd: %d, sn: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    box->cmd, box->sn);
        }
    }

    template<class BoxType>
    void Service<BoxType>::_onMainThreadReceiveMessage(Message<BoxType> *msg) {
        if (!msg) {
            cocos2d::log("[%s]-[%s][%d][%s] null msg", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__);
            return;
        }
        if (!m_delegate) {
            cocos2d::log("[%s]-[%s][%d][%s] null delegate", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__);
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
                cocos2d::log("[%s]-[%s][%d][%s] msg.what: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                        msg->what);
        }

        // 不清除box
        delete msg;
    }

    template<class BoxType>
    void Service<BoxType>::_registerMainThreadSchedule() {
        auto func = [this](float dt){
            Message<BoxType>* message = nullptr;

            // 只要有数据就拼命循环完
            while (1) {
                int ret = this->m_msgQueueFromServer->pop_nowait(message);
                if (ret == 0) {
                    this->_onMainThreadReceiveMessage(message);
                }
                else {
                    break;
                }
            }
        };

        cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, "ferry_ui_loop");
    }

    template<class BoxType>
    void Service<BoxType>::_clearMsgQueues() {

        Message<BoxType>* message = nullptr;
        BoxType* box = nullptr;

        while (m_msgQueueFromServer && m_msgQueueFromServer->pop_nowait(message) == 0) {
            message->forceRelease();
            delete message;
            message = nullptr;
        }

        while (m_msgQueueToServer && m_msgQueueToServer->pop_nowait(box) == 0) {
            delete box;
            box = nullptr;
        }
    }
}


#endif /* end of include guard: __SERVICE_CPP_20141015151947__ */
