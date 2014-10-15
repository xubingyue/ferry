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

#include "NetService.h"
#import "Delegate.h"

// 等待下次连接时间
#define CONNECT_SLEEP_TIME 1

// 队列的大小
#define MSG_FROM_SERVER_SIZE 100
#define MSG_TO_SERVER_SIZE 100

// LOG TAG
#define FERRY_LOG_TAG   "ferry"

namespace ferry {

    template<class BoxType>
    NetService::NetService() {
        m_enabled = false;
        pthread_mutex_init(&m_enabled_mutex, NULL);
        pthread_cond_init(&m_enabled_cond, NULL);

        m_port = 0;
        m_msgQueueFromServer = new BlockQueue<Message *>(MSG_FROM_SERVER_SIZE);
        m_msgQueueToServer = new BlockQueue<BoxType *>(MSG_TO_SERVER_SIZE);
        m_client = nullptr;

        m_recvingMsgFromServer = false;
        m_sendingMsgToServer = false;

        m_should_connect = false;

        m_sendBox = nullptr;
        m_recvMsg = nullptr;
    }

    template<class BoxType>
    NetService::~NetService() {
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
    }

    template<class BoxType>
    bool NetService::init(Delegate<BoxType> *delegate, std::string host, short port) {
        m_delegate = delegate;
        m_host = host;
        m_port = port;
        return true;
    }

    template<class BoxType>
    void NetService::start() {
        if (m_enabled) {
            // 已经调用过一次，是不能再调用的
            return;
        }
        m_enabled = true;

        // 标记要连接服务器
        connect();
        // 启动线程
        _startThreads();
        // 注册到主线程执行
        _registerMainThreadSchedule();
    }

    template<class BoxType>
    void NetService::stop() {
        m_enabled = false;
    }

    template<class BoxType>
    bool NetService::isConnected() {
        return m_client == nullptr ? true : !m_client->isClosed();
    }

    template<class BoxType>
    int NetService::connect() {
        m_should_connect = true;
        return 0;
    }

    template<class BoxType>
    void NetService::_connectToServer() {
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

    void* NetService::_recvMsgFromServerThreadWorker(void *args) {
        NetService* netService = (NetService*)args;
        netService->_recvMsgFromServer();

        return nullptr;
    }

    void* NetService::_sendMsgToServerThreadWorker(void *args) {
        NetService* netService = (NetService*)args;
        netService->_sendMsgToServer();

        return nullptr;
    }

    template<class BoxType>
    void NetService::_closeConn() {
        if (m_client) {
            m_client->closeStream();
        }
        m_client = nullptr;
    }

    template<class BoxType>
    void NetService::_startThreads() {
        _startRecvMsgFromServerThread();
        _startSendMsgToServerThread();
    }

    template<class BoxType>
    void NetService::_startRecvMsgFromServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &NetService::_recvMsgFromServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    template<class BoxType>
    void NetService::_startSendMsgToServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &NetService::_sendMsgToServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    template<class BoxType>
    void NetService::_recvMsgFromServer() {
        int ret;

        m_recvingMsgFromServer = true;

        while (m_enabled) {

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

        m_recvingMsgFromServer = false;
    }

    template<class BoxType>
    void NetService::_sendMsgToServer() {
        m_sendingMsgToServer = true;

        while (m_enabled) {
            int ret = m_msgQueueToServer->pop(m_sendBox);

            if (ret == 0) {
                if (m_client && m_sendBox) {
                    m_client->write(m_sendBox);
                    delete m_sendBox;
                    m_sendBox = nullptr;
                }
            }
        }

        m_sendingMsgToServer = false;
    }

    template<class BoxType>
    void NetService::_onConnOpen() {
        Message * msg = new Message();
        msg->what = DELEGATE_MSG_OPEN;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void NetService::_onConnClose() {
        // 设置为不重连，等触发connectToServer再改状态
        m_should_connect = false;

        Message * msg = new Message();
        msg->what = DELEGATE_MSG_CLOSE;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void NetService::_onMessageFromServer(BoxType* box) {
        Message * msg = new Message();
        msg->what = DELEGATE_MSG_RECV;
        msg->box = box;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
        }
    }

    template<class BoxType>
    void NetService::_onMainThreadReceiveMessage(Message *msg) {
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
                m_delegate->OnOpen(this);
                break;
            case DELEGATE_MSG_RECV:
                m_delegate->OnMessage(this, msg->box);
                break;
            case DELEGATE_MSG_CLOSE:
                m_delegate->OnClose(this);
                break;
            default:
                cocos2d::log("[%s]-[%s][%d][%s] msg.what: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                        msg->what);
        }

        // 不清除box
        delete msg;
        msg = nullptr;
    }

    template<class BoxType>
    void NetService::_registerMainThreadSchedule() {
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
