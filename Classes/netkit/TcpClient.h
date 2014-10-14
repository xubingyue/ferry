//
// Created by dantezhu on 14-8-13.
// Copyright (c) 2014 dantezhu. All rights reserved.
//


#ifndef TCPCLIENT_H_20140814144656
#define TCPCLIENT_H_20140814144656


#include <iostream>
#include "IBox.h"
#include "Stream.h"

namespace netkit {

class TcpClient {
public:
    // timeout <=0 ����������ʱ
    TcpClient(const std::string &host, short port, double timeout);
    ~TcpClient();

    int connectServer();

    // 0 Ϊ�ɹ�
    int read(IBox* box);

    // 0 Ϊ�ɹ�
    int write(IBox* box);
    int write(const char* buf, int bufLen);

    void closeStream();

    bool isClosed();


private:
    std::string m_host;
    short m_port;
    double m_timeout;

    Stream *m_stream;
};

}

#endif //__TcpClient_H_
