//
// Created by dantezhu on 14-10-14.
//


#ifndef __Message_2014_10_14_H_
#define __Message_2014_10_14_H_

#include <iostream>
#include <map>
#include "EventBus.h"
#include "IBox.h"

// 析构的时候，不释放box
class Message {
public:
    Message() {
        box = nullptr;
        what = 0;
    }

public:
    int what;
    netkit::IBox *box;
};


#endif //__Message_2014_10_14_H_
