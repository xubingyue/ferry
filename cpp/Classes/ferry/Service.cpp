//
// Created by dantezhu on 14-10-14.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <fcntl.h>

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

#include <winsock2.h>
#pragma comment(lib,"pthreadVSE2.lib")
#define FERRY_SLEEP(sec) Sleep((sec)*1000);

#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define FERRY_SLEEP(sec) sleep(sec);
#endif

#include "Service.h"
#include "Delegate.h"


namespace ferry {
    void sigTermHandler(int signum) {
        // 早期版本，每次信号处理完之后，随即将信号动作复置为默认值
        signal(signum, (void (*)(int))sigTermHandler);
        if (signum == SIGTERM) {
            pthread_exit(NULL);
        }
    }

    Service::Service() {
        m_running = false;

        m_tryConnectInterval = TRY_CONNECT_INTERVAL;
        m_connectTimeout = CONNECT_TIMEOUT;

        m_port = 0;
        m_msgQueueToServer = new BlockQueue<netkit::IBox *>(MSG_QUEUE_TO_SERVER_MAX_SIZE);
        m_client = nullptr;

        m_shouldConnect = false;
        m_lastActiveTime = 0;

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
    netkit::Stream::startupSocket();
#endif

    }

    Service::~Service() {
        // 线程会有问题，最好的方式是不析构
        stop();

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

    int Service::init(Delegate *delegate, const std::string& host, int port) {
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

        _stopThreads();
        _closeConn();
        _clearMsgQueueToServer();
    }

    bool Service::isConnected() {
        return m_client && !m_client->isClosed();
    }

    bool Service::isRunning() {
        return m_running;
    }

    void Service::connect() {
        m_shouldConnect = true;
    }

    void Service::disconnect() {
        if (m_client) {
            m_client->shutdown(2);
        }
    }

    unsigned int Service::getLastActiveTime() {
        return (unsigned int)m_lastActiveTime;
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

    void Service::setConnectTimeout(int timeout) {
        m_connectTimeout = timeout;
    }

    void Service::_connectToServer() {
        _closeConn();
        // 没有超时

        struct sockaddr_in serv_addr;
        struct in_addr ip_addr;
        ip_addr.s_addr = inet_addr(m_host.c_str());

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port   = htons((unsigned short)m_port);
        serv_addr.sin_addr = ip_addr;

        netkit::SocketType sockFd;

        // 默认就是ERROR
        int connectResult = EVENT_ERROR;

        sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockFd > 0) {

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

            // windows 下的异步代码写起来实在太麻烦了，先暂时这么顶一下

            if (::connect(sockFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0) {
                // 成功了
                connectResult = 0;
            }

# else

            int backupValue = fcntl(sockFd, F_GETFD, 0);

            // 设置为非阻塞
            fcntl(sockFd, F_SETFL, backupValue|O_NONBLOCK);

            if (::connect(sockFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {

                // 需要判断errno
                if (errno == EINPROGRESS) {
                    // 进行中，准备用select判断超时
                    struct timeval tvTimeout;
                    tvTimeout.tv_sec = m_connectTimeout;
                    tvTimeout.tv_usec = 0;
                    
                    fd_set writeFDs;
                    FD_ZERO(&writeFDs);
                    FD_SET(sockFd, &writeFDs);
                    
                    if(select(sockFd + 1, NULL, &writeFDs, NULL, &tvTimeout) > 0){
                        // 说明找到了
                        int tmpError = 0;
                        socklen_t tmpLen = sizeof(tmpError);
                        //下面的一句一定要，主要针对防火墙
                        getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &tmpError, &tmpLen);
                        if(tmpError==0) {
                            // 成功
                            connectResult = 0;
                        }
                    }
                    else {
                        // 超时了没返回
                        connectResult = EVENT_TIMEOUT;
                    }
                }
            }
            else {
                // 成功
                connectResult = 0;
            }
            
            // 恢复到原来的设置
            fcntl(sockFd, F_SETFL, backupValue);

#endif
            
            if (connectResult != 0) {
                CLOSE_SOCKET(sockFd);
            }
        }

        if (connectResult == 0) {
            if (m_client) {
                m_client->setSockFd(sockFd);
            }
            else {
                m_client = new netkit::Stream(sockFd);
            }

            // 分发连接成功的消息
            _onConnOpen();
        }
        else if (connectResult == EVENT_TIMEOUT) {
            // 连接超时了
            // 没关系，下个循环还会继续重连
            _onTimeout();
        }
        else {
            // 连接失败了
            // 没关系，下个循环还会继续重连
            _onError(ERROR_OPEN, nullptr);
        }
    }

    void Service::_closeConn() {
        // 不要清空，直接走到报错回调逻辑里去
        // _clearMsgQueueToServer();

        if (m_client) {
            m_client->close();
        }
    }

    void* Service::_threadWorkerProxy(void *args) {
        signal(SIGTERM,(void (*)(int))sigTermHandler);

        std::pair<Service*, int> *params = (std::pair<Service*, int>*) args;
        switch (params->second) {
            case 0:
                params->first->_recvMsgFromServer();
                break;
            case 1:
                params->first->_sendMsgToServer();
                break;
            
            default:
                break;
        }

        delete params;

        return NULL;
    }

    void Service::_startThreads() {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int ret;
        for (size_t i = 0; i < 2; ++i) {
            ret= pthread_create(&m_recvThread, &attr, &Service::_threadWorkerProxy, (void *) (new std::pair<Service*, int>(this, i)));
            if(ret!=0){
                //ERROR_LOG("Thread creation failed:%d",ret);
            }
        }

        pthread_attr_destroy (&attr);
    }

    void Service::_stopThreads() {
        if (pthread_kill(m_recvThread, 0) == 0) {
            pthread_kill(m_recvThread, SIGTERM);
        }

        if (pthread_kill(m_sendThread, 0) == 0) {
            pthread_kill(m_sendThread, SIGTERM);
        }
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
                // 防止内存泄漏
                m_delegate->releaseBox(box);
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
        m_lastActiveTime = time(NULL);
        m_delegate->onOpen(this);
    }

    void Service::_onConnClose() {
        m_lastActiveTime = 0;
        // 设置为不重连，等触发connectToServer再改状态
        m_shouldConnect = false;
        m_delegate->onClose(this);
    }

    void Service::_onSendMsgToServer(netkit::IBox *box) {
        m_delegate->onSend(this, box);
    }

    void Service::_onRecvMsgFromServer(netkit::IBox *box) {
        m_lastActiveTime = time(NULL);
        m_delegate->onRecv(this, box);
    }

    void Service::_onError(int code, netkit::IBox *ibox) {
        m_delegate->onError(this, code, ibox);
    }

    void Service::_onTimeout() {
        m_delegate->onTimeout(this);
    }

    void Service::_clearMsgQueueToServer() {
        netkit::IBox* box = nullptr;

        while (m_msgQueueToServer && m_msgQueueToServer->pop_nowait(box) == 0) {
            _onError(ERROR_SEND, box);
        }
    }
}
