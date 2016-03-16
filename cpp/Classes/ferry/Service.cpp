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
#define SOCKET_OPT_LEN_TYPE int
// 不能用 char*，否则编译不过
#define SOCKET_OPT_VAL_PTR_TYPE char

#else

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#define FERRY_SLEEP(sec) sleep(sec);
#define SOCKET_OPT_LEN_TYPE socklen_t
#define SOCKET_OPT_VAL_PTR_TYPE void

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

        netkit::SocketType sockFd;

        int connectResult = _customConnect(m_host, m_port, m_connectTimeout, sockFd);

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
    
    void Service::_setBlockSocket(netkit::SocketType sockFd, bool block) {
#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
        u_long mode = block ? 0 : 1;
        
        ioctlsocket(sockFd, FIONBIO, &mode);
#else
        
        int flags = fcntl(sockFd, F_GETFL, 0);
        if (block) {
            // 设置为阻塞
            fcntl(sockFd, F_SETFL, flags & ~O_NONBLOCK);
        }
        else {
            // 设置为非阻塞
            fcntl(sockFd, F_SETFL, flags | O_NONBLOCK);
        }
#endif
    }

    int Service::_customConnect(std::string host, int port, int timeout, netkit::SocketType &resultSock) {
#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
        return _selectConnect(host, port, timeout, resultSock);
#else
        return _pollConnect(host, port, timeout, resultSock);
#endif
    }
    
    int Service::_blockConnect(std::string host, int port, netkit::SocketType &resultSock) {
        // 默认就是ERROR
        int connectResult = EVENT_ERROR;

        struct sockaddr_in serverAddress;
        struct in_addr ipAddress;
        ipAddress.s_addr = inet_addr(host.c_str());

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port   = htons((unsigned short)port);
        serverAddress.sin_addr = ipAddress;
        
        netkit::SocketType sockFd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockFd > 0) {
            if (::connect(sockFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == 0) {
                connectResult = 0;
            }
            else {
                // 失败了
                CLOSE_SOCKET(sockFd);
            }
        }

        // 成功了之后才赋值
        if (connectResult == 0) {
            resultSock = sockFd;
        }

        return connectResult;
    }

    int Service::_selectConnect(std::string host, int port, int timeout, netkit::SocketType &resultSock) {
        // 默认就是ERROR
        int connectResult = EVENT_ERROR;

        struct sockaddr_in serverAddress;
        struct in_addr ipAddress;
        ipAddress.s_addr = inet_addr(host.c_str());

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port   = htons((unsigned short)port);
        serverAddress.sin_addr = ipAddress;
        
        netkit::SocketType sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockFd > 0) {
            
            // 设置为非阻塞
            _setBlockSocket(sockFd, false);
            
            if (::connect(sockFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
                
                // host不存在和port不存在的最大区别时，client发出syn请求后，对方是否有响应.
                // host不存在，syn没有响应
                // port不存在, syn报错
                // linux 默认connect 超时是75秒
#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
                // windows 下不会设置errno，所以无法根据errno判断
                // host/port不存在 都会connect报错，然后进入select
                // host不存在会等待至select超时. onTimeout
                // port不存在会进入errorFDs判断，所以会很快报错. onError
                
                // 注意: 打印errno的时候，要存起来之后再打印，否则可能两次打印会不一致
#else
                // 其他平台需要判断errno
                // host不存在或者连接进行中会进入到 EINPROGRESS. onTimeout
                // port不存在errno会是别的错误，直接报错. onError
                if (errno == EINPROGRESS)
#endif
                    
                {
                    // 进行中，准备用select判断超时
                    struct timeval tvTimeout;
                    tvTimeout.tv_sec = timeout;
                    tvTimeout.tv_usec = 0;
                    
                    // 可写，和报错状态都检测
                    fd_set writeFDs, errorFDs;
                    FD_ZERO(&writeFDs);
                    FD_SET(sockFd, &writeFDs);

                    FD_ZERO(&errorFDs);
                    FD_SET(sockFd, &errorFDs);
                    
                    // windows 下，第一个参数无用，select没有fd数量和fd大小的限制
                    // linux下，fd的个数和最大值都不能超过1024
                    int ret = select(sockFd + 1, NULL, &writeFDs, &errorFDs, &tvTimeout);
                    if(ret > 0){
                        if (FD_ISSET(sockFd, &errorFDs)) {
                            // 报错了
                        }
                        else if (FD_ISSET(sockFd, &writeFDs)) {
                            // 说明找到了
                            int tmpError = 0;
                            SOCKET_OPT_LEN_TYPE tmpLen = sizeof(tmpError);
                            SOCKET_OPT_VAL_PTR_TYPE* ptrTmpError = (SOCKET_OPT_VAL_PTR_TYPE*) &tmpError;

                            // 下面的一句一定要，主要针对防火墙
                            getsockopt(sockFd, SOL_SOCKET, SO_ERROR, ptrTmpError, &tmpLen);

                            if(tmpError==0) {
                                // 成功
                                connectResult = 0;
                            }
                        }
                    }
                    else if (ret == 0) {
                        // 超时了没返回
                        connectResult = EVENT_TIMEOUT;
                    }
                }
            }
            else {
                // 成功
                connectResult = 0;
            }
            
            // 重新设置为阻塞
            _setBlockSocket(sockFd, true);
            
            if (connectResult != 0) {
                CLOSE_SOCKET(sockFd);
            }
        }

        // 成功了之后才赋值
        if (connectResult == 0) {
            resultSock = sockFd;
        }

        return connectResult;
    }

#if !defined(_WIN32) && !(defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

    int Service::_pollConnect(std::string host, int port, int timeout, netkit::SocketType &resultSock) {
        // windows 下没有poll

        // 默认就是ERROR
        int connectResult = EVENT_ERROR;

        struct sockaddr_in serverAddress;
        struct in_addr ipAddress;
        ipAddress.s_addr = inet_addr(host.c_str());

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port   = htons((unsigned short)port);
        serverAddress.sin_addr = ipAddress;
        
        netkit::SocketType sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockFd > 0) {
            
            // 设置为非阻塞
            _setBlockSocket(sockFd, false);
            
            if (::connect(sockFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
                
                if (errno == EINPROGRESS) {
                    
                    struct pollfd fdList[1];
                    
                    struct pollfd &pollSock = fdList[0];
                    
                    memset(&pollSock, 0, sizeof(pollSock));
                    pollSock.fd = sockFd;
                    // 要可写
                    pollSock.events = POLLOUT | POLLERR | POLLHUP;

                    int ret = ::poll(fdList, 1, timeout * 1000);
                    if(ret > 0){
                        // 说明找到了
                        if ((pollSock.revents & POLLERR) || (pollSock.revents & POLLHUP)) {
                            // 报错了
                        }
                        else if (pollSock.revents & POLLOUT) {
                            int tmpError = 0;
                            SOCKET_OPT_LEN_TYPE tmpLen = sizeof(tmpError);
                            SOCKET_OPT_VAL_PTR_TYPE* ptrTmpError = (SOCKET_OPT_VAL_PTR_TYPE*) &tmpError;

                            // 下面的一句一定要，主要针对防火墙
                            getsockopt(sockFd, SOL_SOCKET, SO_ERROR, ptrTmpError, &tmpLen);

                            if(tmpError==0) {
                                // 成功
                                connectResult = 0;
                            }

                        }
                    }
                    else if (ret == 0) {
                        // 超时了没返回
                        connectResult = EVENT_TIMEOUT;
                    }
                }
            }
            else {
                // 成功
                connectResult = 0;
            }
            
            // 重新设置为阻塞
            _setBlockSocket(sockFd, true);
            
            if (connectResult != 0) {
                CLOSE_SOCKET(sockFd);
            }
        }

        // 成功了之后才赋值
        if (connectResult == 0) {
            resultSock = sockFd;
        }

        return connectResult;
    }

#endif
}
