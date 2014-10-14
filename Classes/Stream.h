﻿//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//


#ifndef __Stream_H_
#define __Stream_H_

#include "IBox.h"

namespace netkit {

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
#include <winsock2.h>
typedef SOCKET SocketType;

#define CLOSE_SOCKET    closesocket

#else
typedef int SocketType;

#define CLOSE_SOCKET    close
#endif

class Stream {
public:
    static const int MAX_READ_BUF_SIZE = 640 * 1000;
    static const int RET_RECV_TIMEOUT = -100;


private:
    int mSockFd;
    char* mReadBuffer;
    int mMaxReadBufSize;
    // 实际长度
    int bufferedLength;

private:
    void _init(SocketType sockFd, int maxReadBufSize);

public:
    Stream(SocketType sockFd);
    Stream(SocketType sockFd, int maxReadBufSize);

    // 0 为成功
    int read(IBox* box);

    // 0 为成功
    int write(IBox* box);
    int write(const char* buf, int bufLen);

    void closeStream();

    bool isClosed();

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

    //这两个函数只有在windows下面才用
    //初始化网络模块，一定不能忘，在main函数里面调用一次就可以了。
    static int startupSocket()
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int ret;
        //WinSock初始化
        wVersionRequested = MAKEWORD(1, 1); //希望使用的WinSock DLL的版本
        ret = WSAStartup(wVersionRequested, &wsaData);
        if(ret!=0)
        {
            //printf("WSAStartup() failed!\n");
            return -1;
        }
        //确认WinSock DLL支持版本2.2
        if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1)
        {
            WSACleanup();
            //printf("Invalid WinSock version!\n");
            return -2;
        }

        return 0;
    }
    //这个在游戏退出的时候调用就可以了
    static void cleanupSocket()
    {
        WSACleanup();
    }

#endif
};

}


#endif //__Stream_H_
