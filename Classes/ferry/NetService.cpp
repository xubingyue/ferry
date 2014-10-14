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

// 连接等待时间
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
        m_port = 0;
        m_msgFromServer = new BlockQueue<MsgEvent*>(MSG_FROM_SERVER_SIZE);
        m_msgToServer = new BlockQueue<MsgEvent*>(MSG_TO_SERVER_SIZE);
        m_client = nullptr;

        m_recvingMsgFromServer = false;
        m_sendingMsgToServer = false;

        m_should_connect = false;
    }

    template<class BoxType>
    NetService::~NetService() {
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

        connectToServer();
        _startThreads();
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
    int NetService::connectToServer() {
        m_should_connect = true;
        return 0;
    }

    template<class BoxType>
    void NetService::_connect() {
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
    void NetService::_closeConn() {
        if (m_client) {
            m_client->closeStream();
        }
        m_client = nullptr;
    }

    template<class BoxType>
    void NetService::_startThreads() {
    }


    template<class BoxType>
    void NetService::_recvMsgFromServer() {
        int ret;

        m_recvingMsgFromServer = true;

        while (m_enabled) {

            if (!isConnected()) {
                if (m_should_connect) {
                    // 连接服务器
                    _connect();
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

        }

        m_sendingMsgToServer = false;
    }

    template<class BoxType>
    void NetService::_onConnOpen() {
        if (m_delegate) {
            m_delegate->OnOpen(this);
        }
    }

    template<class BoxType>
    void NetService::_onConnClose() {
        // 设置为不重连，等触发connectToServer再改状态
        m_should_connect = false;

        if (m_delegate) {
            m_delegate->OnClose(this);
        }
    }

    template<class BoxType>
    void NetService::_onMessageFromServer(BoxType* box) {
        if (m_delegate) {
            m_delegate->OnMessage(this, box);
        }
    }
}