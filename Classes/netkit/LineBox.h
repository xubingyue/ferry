//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//

#ifndef LINEBOX_H_20140814144636
#define LINEBOX_H_20140814144636


#include "IBox.h"
#include <iostream>

namespace netkit {


class LineBox: public IBox {

public:
    static const int LINE_END = 10;

private:
    // ����
    std::string _body;

    // ���������ϻᱻ����Ϊtrue
    bool _unpack_done;

public:
    LineBox();

    virtual int pack(char* buf, int maxsize);

    virtual int unpack(const char* buf, int length);

    virtual int check(const char* buf, int length);

    virtual int packetLen();

    const char* getBody();
    const std::string& getStringBody();
    void setBody(const char* p_body, int body_len);

    int headerLen();
    int bodyLen();

    bool unpackDone();
    std::string toString();

private:
    //>0: �ɹ�����obj��������ʹ�õĳ��ȣ���ʣ��Ĳ���bufҪ������
    //<0: ����
    //0: ������
    int _unpack(const char* buf, int length, bool save);
};

}

#endif //__LineBox_H_
