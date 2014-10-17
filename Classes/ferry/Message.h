//
// Created by dantezhu on 14-10-14.
//


#ifndef __Message_2014_10_14_H_
#define __Message_2014_10_14_H_

#include <iostream>
#include <map>
#include "IBox.h"

class Message {
public:
    Message() {
        what = 0;
        box = nullptr;
    }

    void forceRelease() {
        // 强制清空资源
        if (box) {
            delete box;
            box = nullptr;
        }
    }

public:
    int what;
    netkit::IBox *box;
};


#endif //__Message_2014_10_14_H_
