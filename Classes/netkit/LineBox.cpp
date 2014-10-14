//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//

#include <sstream>
#include <string.h>
#include "LineBox.h"

namespace netkit {

LineBox::LineBox() {
    this->_unpack_done = false;
}

int LineBox::pack(char* buf, int maxsize) {
    if (maxsize < this->packetLen()) {
        return -1;
    }

    memcpy(buf, this->getBody(), this->bodyLen());

    return this->packetLen();
}

int LineBox::unpack(const char* buf, int length) {
    return this->_unpack(buf, length, true);
}

int LineBox::check(const char* buf, int length) {
    return this->_unpack(buf, length, false);
}

const char* LineBox::getBody() {
    return this->_body.c_str();
}

const std::string& LineBox::getStringBody() {
    return this->_body;
}

void LineBox::setBody(const char* p_body, int body_len) {
    if (!p_body) {
        this->_body.resize(0);
        return;
    }

    if (p_body[body_len-1] == this->LINE_END) {
        // ���ö�������
        this->_body.resize(body_len);
        // ǿ��д������
        memcpy((void*)this->_body.c_str(), p_body, body_len);
    }
    else {
        // ���ö�������
        this->_body.resize(body_len + 1);
        // ǿ��д������
        memcpy((void*)this->_body.c_str(), p_body, body_len);

        ((char*)this->_body.c_str())[body_len] = this->LINE_END;
    }
}

int LineBox::headerLen() {
    return 0;
}
int LineBox::bodyLen() {
    return this->_body.size();
}
int LineBox::packetLen() {
    return this->headerLen() + this->bodyLen();
}

bool LineBox::unpackDone() {
    return this->_unpack_done;
}

int LineBox::_unpack(const char* buf, int length, bool save) {
    if (!buf) {
        return -1;
    }

    for (int i=0; i < length; i++) {
        char c = buf[i];
        if (c == LINE_END) {
            int pktLen_ = i + 1;
            if (!save) {
                return pktLen_;
            }

            this->setBody(buf, i+1);

            this->_unpack_done = true;
            return pktLen_;
        }
    }

    // ������
    return 0;
}

std::string LineBox::toString() {
    std::stringstream ss;
    ss << "bodyLen: " << this->bodyLen();

    return ss.str();
}

}
