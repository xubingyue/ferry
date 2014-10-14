//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//


#ifndef BOX_H_20140814144610
#define BOX_H_20140814144610

#include "IBox.h"

namespace netkit {

class Box: public IBox {
public:
    // ħ������
    static const int MAGIC = 2037952207;

    // Ĭ�Ͼ������ֵ
    int magic;
    int version;
    // ���ֻ�����������ϴ��䣬������������������
    int _transfer_packet_len;
    int cmd;
    int ret;
    int sn;

private:
    // ����
    std::string _body;

    // ���������ϻᱻ����Ϊtrue
    bool _unpack_done;

public:
    Box();

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

#endif //__Box_H_
