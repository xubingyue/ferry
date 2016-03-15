//
// Created by dantezhu on 14-10-14.
//

#ifndef __SERVICE_H_20141014200634__
#define __SERVICE_H_20141014200634__

#include <iostream>
#include <pthread.h>
#include "BlockQueue.h"
#include "netkit/Stream.h"
#include "netkit/IBox.h"
#include "Constants.h"

namespace ferry {

class Delegate;

class Service {

public:
    Service();
    virtual ~Service();

    // 初始化
    int init(Delegate *delegate, const std::string& host, int port);
    // 启动
    void start();
    // 停止，一般在游戏结束时
    void stop();

    // 连接到服务器，如果断线重连要调用这个
    void connect();

    // 主动关闭连接，比如认为连接不正常或者要切换host时
    // 可以确保触发onClose
    void disconnect();

    // 是否连接成功
    bool isConnected();

    // 是否运行中
    bool isRunning();

    // 最近一次onOpen或onRecv的时间
    unsigned int getLastActiveTime();

    // 发送消息
    void send(netkit::IBox* box);

    // 设置消息队列大小
    void setMsgQueueToServerMaxSize(int maxsize);

    // 设置连接失败后的重连间隔
    void setTryConnectInterval(int interval);

    // 设置连接超时
    void setConnectTimeout(int timeout);

private:
    // 为了启动线程使用的，外面不要使用
    static void* _threadWorkerProxy(void *args);

    // 真实连接
    void _connectToServer();

    // 真实关闭连接
    void _closeConn();

    // 启动线程
    void _startThreads();
    // 关闭线程
    void _stopThreads();

    // 从网络获取消息
    void _recvMsgFromServer();
    // 从本地获取消息
    void _sendMsgToServer();

    // 当连接建立
    void _onConnOpen();

    // 当连接关闭
    void _onConnClose();

    // 当向服务器发送消息
    void _onSendMsgToServer(netkit::IBox *box);

    // 当收到服务器消息
    void _onRecvMsgFromServer(netkit::IBox *box);

    // 当报错的时候
    void _onError(int code, netkit::IBox *ibox);

    // 当超时的时候
    void _onTimeout();

    void _clearMsgQueueToServer();
    
    // 设置阻塞/非阻塞socket
    void _setBlockSocket(netkit::SocketType sockFd, bool block);

    // 自定义连接，内部可能使用以下3种连接
    int _customConnect(std::string host, int port, int timeout,
                       netkit::SocketType &resultSock);

    // 同步连接
    int _blockConnect(std::string host, int port,
                       netkit::SocketType &resultSock);
    
    // 通过select异步连接
    int _selectConnect(std::string host, int port, int timeout,
                       netkit::SocketType &resultSock);

#if !defined(_WIN32) && !(defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
    // 通过poll异步连接
    int _pollConnect(std::string host, int port, int timeout,
                       netkit::SocketType &resultSock);
#endif


private:
    // 是否运行中
    bool m_running;

    // 连接失败后的重连间隔
    int m_tryConnectInterval;
    
    // 连接超时
    int m_connectTimeout;

    Delegate *m_delegate;
    std::string m_host;
    int m_port;

    time_t m_lastActiveTime;

    // 是否要尝试连接，因为有时候会要提示用户连接断开
    // 用户点击重试才继续尝试
    bool m_shouldConnect;

    // 想要发送给server的消息
    BlockQueue<netkit::IBox *> *m_msgQueueToServer;

    // 网络
    netkit::Stream *m_client;

    pthread_t m_recvThread;
    pthread_t m_sendThread;
};

}

#endif /* end of include guard: __SERVICE_H_20141014200634__ */
