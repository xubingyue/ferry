﻿//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//

#include <sstream>
#include <string.h>
#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif
#include "Box.h"

#define box_mcp(dst, value) \
    memcpy(dst, &(value), sizeof(value)); \
    dst += sizeof(value);

namespace netkit {

Box::Box() {
    this->magic = MAGIC;
    this->version = 0;
    this->_transfer_packet_len = 0;
    this->cmd = 0;
    this->ret = 0;
    this->sn = 0;
    this->_unpack_done = false;
}

int Box::pack(char* buf, int maxsize) {
    if (maxsize < this->packetLen()) {
        // 说明包长都不够
        return -1;
    }

    int temp;

    temp = htonl(this->magic);
    box_mcp(buf, temp);

    temp = htonl(this->version);
    box_mcp(buf, temp);

    temp = htonl(this->packetLen());
    box_mcp(buf, temp);

    temp = htonl(this->cmd);
    box_mcp(buf, temp);

    temp = htonl(this->ret);
    box_mcp(buf, temp);

    temp = htonl(this->sn);
    box_mcp(buf, temp);

    memcpy(buf, this->getBody(), this->bodyLen());
    buf += this->bodyLen();

    return this->packetLen();
}

int Box::unpack(const char* buf, int length) {
    return this->_unpack(buf, length, true);
}


int Box::check(const char* buf, int length) {
    return this->_unpack(buf, length, false);
}

int Box::_unpack(const char* buf, int length, bool save) {
    if (length < this->headerLen()) {
        // 连包头都不够
        return 0;
    }

    int magic_ = ntohl(*((int*)buf));
    buf += sizeof(int);

    if (magic_ != MAGIC) {
        // 说明magic不对
        return -1;
    }

    int version_ = ntohl(*((int*)buf));
    buf += sizeof(int);

    int pktLen_ = ntohl(*((int*)buf));
    buf += sizeof(int);

    int cmd_ = ntohl(*((int*)buf));
    buf += sizeof(int);

    int ret_ = ntohl(*((int*)buf));
    buf += sizeof(int);

    int sn_ = ntohl(*((int*)buf));
    buf += sizeof(int);

    if (pktLen_ > length) {
        // 还没收完，继续
        return 0;
    }

    if (!save) {
        return pktLen_;
    }

    this->magic = magic_;
    this->version = version_;
    this->_transfer_packet_len = pktLen_;
    this->cmd = cmd_;
    this->ret = ret_;
    this->sn = sn_;

    this->setBody(buf, pktLen_ - this->headerLen());

    this->_unpack_done = true;

    return pktLen_;
}

const char* Box::getBody() {
    return this->_body.c_str();
}

const std::string& Box::getStringBody() {
    return this->_body;
}

void Box::setBody(const char* p_body, int body_len) {
    if (!p_body) {
        this->_body.resize(0);
        return;
    }

    this->_body.resize(body_len);
    // 强制写入数据
    memcpy((void*)this->_body.c_str(), p_body, body_len);
}

int Box::headerLen() {
    return sizeof(this->magic) + sizeof(this->version) + sizeof(this->_transfer_packet_len)
        + sizeof(this->cmd) + sizeof(this->ret) + sizeof(this->sn);
}

int Box::bodyLen() {
    return this->_body.size();
}

int Box::packetLen() {
    return this->headerLen() + this->bodyLen();
}

bool Box::unpackDone() {
    return this->_unpack_done;
}

std::string Box::toString() {
    std::stringstream ss;
    ss << "magic: " << this->magic;
    ss << ", version: " << this->version;
    ss << ", cmd: " << this->cmd;
    ss << ", ret: " << this->ret;
    ss << ", sn: " << this->sn;
    ss << ", bodyLen: " << this->bodyLen();

    return ss.str();
}

}
