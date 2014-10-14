//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//


#ifndef STREAM_H_20140814144646
#define STREAM_H_20140814144646


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
    // ʵ�ʳ���
    int bufferedLength;

private:
    void _init(SocketType sockFd, int maxReadBufSize);

public:
    Stream(SocketType sockFd);
    Stream(SocketType sockFd, int maxReadBufSize);

    // 0 Ϊ�ɹ�
    int read(IBox* box);

    // 0 Ϊ�ɹ�
    int write(IBox* box);
    int write(const char* buf, int bufLen);

    void closeStream();

    bool isClosed();

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

    //����������ֻ����windows�������
    //��ʼ������ģ�飬һ������������main�����������һ�ξͿ����ˡ�
    static int startupSocket()
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int ret;
        //WinSock��ʼ��
        wVersionRequested = MAKEWORD(1, 1); //ϣ��ʹ�õ�WinSock DLL�İ汾
        ret = WSAStartup(wVersionRequested, &wsaData);
        if(ret!=0)
        {
            //printf("WSAStartup() failed!\n");
            return -1;
        }
        //ȷ��WinSock DLL֧�ְ汾2.2
        if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1)
        {
            WSACleanup();
            //printf("Invalid WinSock version!\n");
            return -2;
        }

        return 0;
    }
    //�������Ϸ�˳���ʱ����þͿ�����
    static void cleanupSocket()
    {
        WSACleanup();
    }

#endif
};

}


#endif //__Stream_H_
