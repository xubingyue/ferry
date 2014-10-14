//
// Created by dantezhu on 14-7-26.
// Copyright (c) 2014 dantezhu. All rights reserved.
//

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)

#include <winsock2.h>

#else

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#endif

#include "Stream.h"

namespace netkit {

void Stream::_init(SocketType sockFd, int maxReadBufSize) {
    this->mReadBuffer = NULL;
    this->bufferedLength = 0;

    this->mSockFd = sockFd;
    this->mMaxReadBufSize = maxReadBufSize > 0 ? maxReadBufSize : MAX_READ_BUF_SIZE;

    this->mReadBuffer = new char[this->mMaxReadBufSize];
}

Stream::Stream(SocketType sockFd) {
    this->_init(sockFd, MAX_READ_BUF_SIZE);
}

Stream::Stream(SocketType sockFd, int maxReadBufSize) {
    this->_init(sockFd, maxReadBufSize);
}

int Stream::read(IBox* box) {
    if (this->isClosed()) {
        return -1;
    }

    if (!box) {
        return -2;
    }

    // 直接只支持
    while(true) {
        if (bufferedLength > 0) {
            // 说明还是可以尝试一下的

            int ret = box->unpack(mReadBuffer, bufferedLength);
            if (ret > 0) {
                // 说明成功

                bufferedLength -= ret;

                memmove(mReadBuffer, mReadBuffer + ret, bufferedLength);

                return 0;
            }
            else if (ret < 0) {
                // 说明数据错乱了
                bufferedLength = 0;
            }
        }

        if (mMaxReadBufSize - bufferedLength <= 0) {
            // 已经没有多余的空间来储存了
            return -2;
        }

        int len = recv(mSockFd, mReadBuffer + bufferedLength, mMaxReadBufSize - bufferedLength, 0);
        // -1：当server关闭的时候会报这个错误
        if (len <= 0) {
            // 说明报错了，或者连接失败了

            if (len == -1 && errno == EAGAIN) {
                // 读取超时
                return RET_RECV_TIMEOUT;
            }
            //printf("len: %d\n, errno: %d, %s", len, errno, strerror(errno), EAGAIN);
            this->closeStream();
            return -3;
        }

        // 进入下个循环自然会判断
        bufferedLength += len;
    }

}

int Stream::write(IBox* box) {
    if (!box) {
        return -1;
    }

    std::string str;
    str.resize(box->packetLen());

    int packetLen = box->pack((char*)str.c_str(), str.size());
    if (!packetLen) {
        return -2;
    }

    return write(str.c_str(), packetLen);
}

int Stream::write(const char* buf, int bufLen) {

    int sentLen = 0;

    while (sentLen < bufLen) {
        int len = send(mSockFd, buf + sentLen, bufLen - sentLen, 0);
        if (len < 0) {
            return -3;
        }

        sentLen += len;
    }

    return 0;
}

void Stream::closeStream() {
    if (mSockFd > 0) {
        CLOSE_SOCKET(mSockFd);
        mSockFd = 0;
    }
}

bool Stream::isClosed() {
    return mSockFd <= 0;
}

}
