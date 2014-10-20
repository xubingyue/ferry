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
    enum ERROR_CODE {
        ERROR_PUSH_OPEN_MSG_TO_QUEUE,
        ERROR_PUSH_SEND_MSG_TO_QUEUE,
        ERROR_PUSH_RECV_MSG_TO_QUEUE,
        ERROR_PUSH_CLOSE_MSG_TO_QUEUE,
    };

    // 等待下次连接时间(秒)
    const int TRY_CONNECT_INTERVAL = 1;

    const int MSG_QUEUE_MAX_SIZE_FROM_SERVER = 100;
    const int MSG_QUEUE_MAX_SIZE_TO_SERVER = 100;

    const std::string COCOS_SCHEDULE_NAME = "ferry_service";


    Service::Service() {
        m_running = false;

        m_tryConnectInterval = TRY_CONNECT_INTERVAL;

        pthread_mutex_init(&m_running_mutex, NULL);
        pthread_cond_init(&m_running_cond, NULL);

        m_port = 0;
        m_msgQueueFromServer = new BlockQueue<Message *>(MSG_QUEUE_MAX_SIZE_FROM_SERVER);
        m_msgQueueToServer = new BlockQueue<netkit::IBox *>(MSG_QUEUE_MAX_SIZE_TO_SERVER);
        m_client = nullptr;

        m_shouldConnect = false;

        m_threadsRunning = false;

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
    netkit::Stream::startupSocket();
#endif

    }

    Service::~Service() {
        // 线程会有问题，最好的方式是不析构
        stop();

        pthread_mutex_destroy(&m_running_mutex);
        pthread_cond_destroy(&m_running_cond);

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

    int Service::init(Delegate *delegate, std::string host, short port) {
        m_delegate = delegate;
        m_host = host;
        m_port = port;
        return 0;
    }

    void Service::start() {
        if (m_running) {
            // 已经调用过一次，是不能再调用的
            return;
        }
        _setRunning(true);

        // 标记要连接服务器
        connect();
        // 启动线程，只要启动一次即可
        if (!m_threadsRunning) {
            _startThreads();
            m_threadsRunning = true;
        }
        // 注册到主线程执行
        _registerMainThreadSchedule();
    }

    void Service::stop() {
        if (!m_running) {
            return;
        }

        // 一定要放到最前面
        _setRunning(false);
        // 关闭连接
        closeConn();

        // 取消主线程定时器
        _unRegisterMainThreadSchedule();
    }

    bool Service::isConnected() {
        return m_client == nullptr ? false : !m_client->isClosed();
    }

    bool Service::isRunning() {
        return m_running;
    }

    void Service::connect() {
        m_shouldConnect = true;
    }

    void Service::closeConn() {
        // 关闭，就要把所有消息先清空
        _clearMsgQueues();

        if (m_client) {
            m_client->closeStream();
            delete m_client;
            m_client = nullptr;
        }
    }

    void Service::send(netkit::IBox *box) {
        // 放到前面，否则很可能box会被析构
        m_delegate->onSend(this, box);

        int ret = m_msgQueueToServer->push_nowait(box);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
            m_delegate->onError(this, ERROR_PUSH_SEND_MSG_TO_QUEUE);
        }
    }

    void Service::setMsgQueueMaxSizeFromServer(int maxsize) {
        m_msgQueueFromServer->setMaxSize(maxsize);
    }

    void Service::setMsgQueueMaxSizeToServer(int maxsize) {
        m_msgQueueToServer->setMaxSize(maxsize);
    }

    void Service::setTryConnectInterval(int interval) {
        m_tryConnectInterval = interval;
    }

    void Service::_setRunning(bool running) {
        pthread_mutex_lock(&m_running_mutex);
        m_running = running;
        if (m_running) {
            // 只通知可用
            pthread_cond_broadcast(&m_running_cond);
        }
        pthread_mutex_unlock(&m_running_mutex);
    }

    void Service::_connectToServer() {
        closeConn();
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

    void* Service::_recvMsgFromServerThreadWorker(void *args) {
        Service* netService = (Service*)args;
        netService->_recvMsgFromServer();

        return nullptr;
    }

    void* Service::_sendMsgToServerThreadWorker(void *args) {
        Service* netService = (Service*)args;
        netService->_sendMsgToServer();

        return nullptr;
    }

    void Service::_startThreads() {
        _startRecvMsgFromServerThread();
        _startSendMsgToServerThread();
    }

    void Service::_startRecvMsgFromServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &Service::_recvMsgFromServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    void Service::_startSendMsgToServerThread() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        pthread_t threadId;
        ret= pthread_create(&threadId, &attr, &Service::_sendMsgToServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    void Service::_recvMsgFromServer() {
        int ret;

        while (1) {
            if (!m_running) {
                pthread_mutex_lock(&m_running_mutex);
                // 锁定后再确认一下
                if (!m_running) {
                    // 只通知可用
                    pthread_cond_wait(&m_running_cond, &m_running_mutex);
                }
                pthread_mutex_unlock(&m_running_mutex);
            }

            if (!isConnected()) {
                if (m_shouldConnect) {
                    // 连接服务器
                    _connectToServer();
                }

                if (!isConnected()) {
                    // 如果还没有建立链接，就等一下
                    FERRY_SLEEP(m_tryConnectInterval);
                }
                continue;
            }

            netkit::IBox* box = m_delegate->allocBox();

            ret = m_client->read(box);
            if (ret < 0) {
                // 统一按照断掉链接处理
                _onConnClose();
            }
            else {
                // 有数据
                _onRecvMsgFromServer(box);
            }
        }
    }

    void Service::_sendMsgToServer() {
        netkit::IBox* box = nullptr;

        int ret;

        while (1) {
            if (!m_running) {
                pthread_mutex_lock(&m_running_mutex);
                // 锁定后再确认一下
                if (!m_running) {
                    // 只通知可用
                    pthread_cond_wait(&m_running_cond, &m_running_mutex);
                }
                pthread_mutex_unlock(&m_running_mutex);
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

    void Service::_onConnOpen() {
        Message * msg = new Message();
        msg->what = DELEGATE_MSG_OPEN;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            cocos2d::log("[%s]-[%s][%d][%s] ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                    ret);
            m_delegate->onError(this, ERROR_PUSH_OPEN_MSG_TO_QUEUE);
        }
    }

    void Service::_onConnClose() {
        // 设置为不重连，等触发connectToServer再改状态
        m_shouldConnect = false;

        Message * msg = new Message();
        msg->what = DELEGATE_MSG_CLOSE;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            m_delegate->onError(this, ERROR_PUSH_CLOSE_MSG_TO_QUEUE);
        }
    }

    void Service::_onRecvMsgFromServer(netkit::IBox *box) {
        Message * msg = new Message();
        msg->what = DELEGATE_MSG_RECV;
        msg->box = box;
        int ret = m_msgQueueFromServer->push_nowait(msg);
        if (ret) {
            m_delegate->onError(this, ERROR_PUSH_RECV_MSG_TO_QUEUE);
        }
    }

    void Service::_onMainThreadReceiveMessage(Message *msg) {
        if (!msg) {
            cocos2d::log("[%s]-[%s][%d][%s] null msg", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__);
            return;
        }

        switch (msg->what) {
            case DELEGATE_MSG_OPEN:
                m_delegate->onOpen(this);
                break;
            case DELEGATE_MSG_RECV:
                m_delegate->onRecv(this, msg->box);
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

    void Service::_registerMainThreadSchedule() {
        auto func = [this](float dt){
            Message* message = nullptr;

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

        cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, COCOS_SCHEDULE_NAME);
    }

    void Service::_unRegisterMainThreadSchedule() {
        cocos2d::Director::getInstance()->getScheduler()->unschedule(COCOS_SCHEDULE_NAME, this);
    }

    void Service::_clearMsgQueues() {
        Message* message = nullptr;
        netkit::IBox* box = nullptr;

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
