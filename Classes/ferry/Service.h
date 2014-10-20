//
// Created by dantezhu on 14-10-14.
//

#ifndef __SERVICE_H_20141014200634__
#define __SERVICE_H_20141014200634__

#include <iostream>
#include <pthread.h>
#include "BlockQueue.h"
#include "Message.h"
#include "TcpClient.h"
#include "IBox.h"

namespace ferry {

class Delegate;

class Service {

public:
    Service();
    virtual ~Service();

    // 初始化
    int init(Delegate *delegate,std::string host, short port);
    // 启动
    void start();
    // 停止，一般在游戏结束时
    void stop();

    // 连接到服务器，如果断线重连要调用这个
    void connect();
    // 主动关闭连接，比如认为连接不正常时
    void closeConn();

    // 是否连接成功
    bool isConnected();

    // 是否运行中
    bool isRunning();

    // 发送消息
    void send(netkit::IBox* box);

    // 设置消息队列大小
    void setMsgQueueMaxSizeFromServer(int maxsize);
    void setMsgQueueMaxSizeToServer(int maxsize);

    // 设置链接失败后的重连间隔
    void setTryConnectInterval(int interval);

    // 为了启动线程使用的，外面不要使用
    static void* _recvMsgFromServerThreadWorker(void *args);
    static void* _sendMsgToServerThreadWorker(void *args);

private:
    // 设置running
    void _setRunning(bool running);
    // 真实连接
    void _connectToServer();

    // 启动各种线程
    void _startThreads();

    // 启动接收线程
    void _startRecvMsgFromServerThread();
    // 启动接收线程
    void _startSendMsgToServerThread();

    // 从网络获取消息
    void _recvMsgFromServer();
    // 从本地获取消息
    void _sendMsgToServer();

    // 当链接建立
    void _onConnOpen();

    // 当链接关闭
    void _onConnClose();

    // 当收到服务器消息
    void _onRecvMsgFromServer(netkit::IBox *box);

    // 主线程的处理
    void _onMainThreadReceiveMessage(Message *msg);

    void _registerMainThreadSchedule();
    void _unRegisterMainThreadSchedule();

    void _clearMsgQueues();


private:
    // 是否运行中
    bool m_running;

    // 连接失败后的重连间隔
    int m_tryConnectInterval;

    pthread_mutex_t m_running_mutex;
    pthread_cond_t m_running_cond;

    Delegate *m_delegate;
    std::string m_host;
    short m_port;

    // 是否要尝试连接，因为有时候会要提示用户链接断开
    // 用户点击重试才继续尝试
    bool m_shouldConnect;

    // 从server读取的消息
    BlockQueue<Message*> *m_msgQueueFromServer;

    // 想要发送给server的消息
    BlockQueue<netkit::IBox *> *m_msgQueueToServer;

    // 网络
    netkit::TcpClient *m_client;

    // 线程是否已经启动
    bool m_threadsRunning;
};

}

#endif /* end of include guard: __SERVICE_H_20141014200634__ */
