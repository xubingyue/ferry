//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//


#ifndef IBOX_H_20140814144626
#define IBOX_H_20140814144626


#include <iostream>

namespace netkit {

class IBox {
public:
    // >=0: buf�ĳ���
    // <0:  ����
    virtual int pack(char* buf, int maxsize)=0;

    virtual int unpack(const char* buf, int length)=0;

    virtual int check(const char* buf, int length)=0;

    virtual int packetLen()=0;
};

}

#endif //__IBox_H_
