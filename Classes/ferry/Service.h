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

namespace ferry {

template<class T>
class Delegate;

template <class BoxType>
class Service {

public:
    Service();
    ~Service();

    // 初始化
    bool init(Delegate<BoxType> *delegate,std::string host, short port);
    // 启动
    void start();
    // 停止，一般在游戏结束时
    void stop();

    // 连接到服务器，如果断线重连要调用这个
    int connect();

    // 是否连接成功
    bool isConnected();

    // 为了启动线程使用的，外面不要使用
    static void* _recvMsgFromServerThreadWorker(void *args);
    static void* _sendMsgToServerThreadWorker(void *args);

private:
    // 设置enabled
    void _setEnabled(bool enabled);
    // 真实连接
    void _connectToServer();
    // 主动关闭连接，比如认为连接不正常时
    void _closeConn();

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
    void _onMessageFromServer(BoxType* box);

    // 主线程的处理
    void _onMainThreadReceiveMessage(Message<BoxType> *msg);

    void _registerMainThreadSchedule();


private:
    // 是否运行中
    bool m_enabled;
    pthread_mutex_t m_enabled_mutex;
    pthread_cond_t m_enabled_cond;

    Delegate<BoxType> *m_delegate;
    std::string m_host;
    short m_port;

    // 是否要尝试连接，因为有时候会要提示用户链接断开
    // 用户点击重试才继续尝试
    bool m_should_connect;

    // 从server读取的消息
    BlockQueue<Message<BoxType> *> *m_msgQueueFromServer;

    // 想要发送给server的消息
    BlockQueue<BoxType *> *m_msgQueueToServer;

    // 网络
    netkit::TcpClient *m_client;

    // 不知道为什么，临时变量不能用
    BoxType* m_sendBox;
    Message<BoxType>* m_recvMsg;
};

}

#endif /* end of include guard: __SERVICE_H_20141014200634__ */
