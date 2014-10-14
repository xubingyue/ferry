//
// Created by dantezhu on 14-10-14.
//

#ifndef __NETSERVICE_H_20141014200634__
#define __NETSERVICE_H_20141014200634__

#include <iostream>
#include "block_queue.h"
#include "MsgEvent.h"
#include "TcpClient.h"

namespace ferry {

template<class T>
class Delegate;

enum DELEGATE_MSG_TYPE {
    DELEGATE_MSG_OPEN=1,
    DELEGATE_MSG_RECV=2,
    DELEGATE_MSG_CLOSE=3,
};

template <class BoxType>
class NetService {

public:
    inline NetService();
    inline ~NetService();

    // 初始化
    inline bool init(Delegate<BoxType> *delegate,std::string host, short port);
    // 启动
    inline void start();
    // 停止，一般在游戏结束时
    inline void stop();

    // 是否连接成功
    inline bool isConnected();

    // 连接到服务器，如果断线重连要调用这个
    inline int connectToServer();

private:
    // 真实连接
    inline void _connect();
    // 主动关闭连接，比如认为连接不正常时
    inline void _closeConn();

    // 启动各种线程
    inline void _startThreads();

    // 从网络获取消息
    inline void _recvMsgFromServer();
    // 从本地获取消息
    inline void _sendMsgToServer();

    // 当链接建立
    inline void _onConnOpen();

    // 当链接关闭
    inline void _onConnClose();

    // 当收到服务器消息
    inline void _onMessageFromServer(BoxType* box);

private:
    // 是否运行中
    bool m_enabled;

    Delegate<BoxType> *m_delegate;
    std::string m_host;
    short m_port;

    // 是否要尝试连接，因为有时候会要提示用户链接断开
    // 用户点击重试才继续尝试
    bool m_should_connect;

    // 从server读取的消息
    BlockQueue<MsgEvent*> *m_msgFromServer;

    // 想要发送给server的消息
    BlockQueue<MsgEvent*> *m_msgToServer;

    // 网络
    netkit::TcpClient *m_client;

    // 线程是否存在
    bool m_recvingMsgFromServer;
    bool m_sendingMsgToServer;
};

}

#endif /* end of include guard: __NETSERVICE_H_20141014200634__ */