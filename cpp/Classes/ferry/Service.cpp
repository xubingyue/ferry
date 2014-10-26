//
// Created by dantezhu on 14-10-14.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <signal.h>
#include "cocos2d.h"

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

#include <winsock2.h>
#pragma comment(lib,"pthreadVSE2.lib")
#define FERRY_SLEEP(sec) Sleep((sec)*1000);

#else
#include <unistd.h>
#include <arpa/inet.h>
#define FERRY_SLEEP(sec) sleep(sec);
#endif

#include "Service.h"
#include "Delegate.h"


namespace ferry {
    void sigTermHandler(int signum) {
        /*早期版本，每次信号处理完之后，随即将信号动作复置为默认值*/
        signal(signum, (void (*)(int))sigTermHandler);
        if (signum == SIGTERM) {
            cocos2d::log("sigal: %d, thread: %u\n", signum, pthread_self());
            pthread_exit(NULL);
        }
    }

    Service::Service() {
        m_running = false;

        m_tryConnectInterval = TRY_CONNECT_INTERVAL;

        m_port = 0;
        m_msgQueueToServer = new BlockQueue<netkit::IBox *>(MSG_QUEUE_TO_SERVER_MAX_SIZE);
        m_client = nullptr;

        m_shouldConnect = false;

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
    netkit::Stream::startupSocket();
#endif

    }

    Service::~Service() {
        // 线程会有问题，最好的方式是不析构
        stop();

        _clearMsgQueueToServer();

        if (m_client) {
            delete m_client;
            m_client = nullptr;
        }

        if (m_msgQueueToServer) {
            delete m_msgQueueToServer;
            m_msgQueueToServer = nullptr;
        }

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
    netkit::Stream::cleanupSocket();
#endif

    }

    int Service::init(Delegate *delegate, const std::string& host, short port) {
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
        m_running = true;

        // 标记要连接服务器
        connect();

        _startThreads();
    }

    void Service::stop() {
        if (!m_running) {
            return;
        }
        m_running = false;

        if (pthread_kill(m_recvThread, 0) == 0) {
            pthread_kill(m_recvThread, SIGTERM);
        }

        if (pthread_kill(m_sendThread, 0) == 0) {
            pthread_kill(m_sendThread, SIGTERM);
        }

        _closeConn();
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

    void Service::disconnect() {
        if (m_client) {
            m_client->shutdownStream(2);
        }
    }

    void Service::_closeConn() {
        // 不要清空，直接走到报错回调逻辑里去
        // _clearMsgQueueToServer();

        if (m_client) {
            m_client->closeStream();
            delete m_client;
            m_client = nullptr;
        }
    }

    void Service::send(netkit::IBox *box) {
        int ret = m_msgQueueToServer->push_nowait(box);
        if (ret) {
            // 由delegate负责释放
            _onError(ERROR_SEND, box);
        }
        // 不再返回值，为了防止调用方以为这里的ret=0就代表发送成功
        // 错误都会回调到 onError
    }

    void Service::setMsgQueueToServerMaxSize(int maxsize) {
        m_msgQueueToServer->setMaxSize(maxsize);
    }

    void Service::setTryConnectInterval(int interval) {
        m_tryConnectInterval = interval;
    }

    void Service::_connectToServer() {
        _closeConn();
        // 没有超时
        m_client = new netkit::TcpClient(m_host, m_port, -1);

        int ret = m_client->connectServer();
        if (ret) {
            // 连接失败了
            // 没关系，下个循环还会继续重连
            _onError(ERROR_OPEN, nullptr);
        }
        else {
            // 分发连接成功的消息
            _onConnOpen();
        }
    }

    void* Service::_recvMsgFromServerThreadWorker(void *args) {
        signal(SIGTERM,(void (*)(int))sigTermHandler);

        Service* netService = (Service*)args;
        netService->_recvMsgFromServer();

        return nullptr;
    }

    void* Service::_sendMsgToServerThreadWorker(void *args) {
        signal(SIGTERM,(void (*)(int))sigTermHandler);

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
        ret= pthread_create(&m_recvThread, &attr, &Service::_recvMsgFromServerThreadWorker, (void *) this);
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
        ret= pthread_create(&m_sendThread, &attr, &Service::_sendMsgToServerThreadWorker, (void *) this);
        if(ret!=0){
            //ERROR_LOG("Thread creation failed:%d",ret);
            pthread_attr_destroy (&attr);
        }

        pthread_attr_destroy (&attr);
    }

    void Service::_recvMsgFromServer() {
        int ret;

        while (m_running) {
            if (!isConnected()) {
                if (m_shouldConnect) {
                    // 连接服务器
                    _connectToServer();
                }

                if (!isConnected()) {
                    // 如果还没有建立连接，就等一下
                    FERRY_SLEEP(m_tryConnectInterval);
                    continue;
                }
            }

            netkit::IBox* box = m_delegate->createBox();

            ret = m_client->read(box);
            if (ret < 0) {
                // 手工先关闭掉
                _closeConn();
                // 统一按照断掉连接处理
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

        while (m_running) {
            // send线程不检查running，这样可以保证要发送的box都会有对应的回调

            ret = m_msgQueueToServer->pop(box);

            if (ret == 0 && box) {
                // 由delegate负责释放box
                if (isConnected()) {
                    ret = m_client->write(box);
                    if (ret == 0) {
                        _onSendMsgToServer(box);
                    }
                    else {
                        _onError(ERROR_SEND, box);
                    }
                }
                else {
                    _onError(ERROR_SEND, box);
                }
            }
        }
    }

    void Service::_onConnOpen() {
        m_delegate->onOpen(this);
    }

    void Service::_onConnClose() {
        // 设置为不重连，等触发connectToServer再改状态
        m_shouldConnect = false;
        m_delegate->onClose(this);
    }

    void Service::_onSendMsgToServer(netkit::IBox *box) {
        m_delegate->onSend(this, box);
    }

    void Service::_onRecvMsgFromServer(netkit::IBox *box) {
        m_delegate->onRecv(this, box);
    }

    void Service::_onError(int code, netkit::IBox *ibox) {
        m_delegate->onError(this, code, ibox);
    }

    void Service::_clearMsgQueueToServer() {
        netkit::IBox* box = nullptr;

        while (m_msgQueueToServer && m_msgQueueToServer->pop_nowait(box) == 0) {
            _onError(ERROR_SEND, box);
        }
    }
}
