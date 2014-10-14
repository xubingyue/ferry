//
// Created by dantezhu on 14-10-14.
//


#ifndef __MsgEvent_2014_10_14_H_
#define __MsgEvent_2014_10_14_H_

#include <iostream>
#include <map>
#include "EventBus.h"
#include "IBox.h"

class MsgEvent :public eventbus::BaseEvent{
public:
    CREATE_FUNC(MsgEvent);

    MsgEvent() {
        box = nullptr;
        what = 0;
    }
    virtual ~MsgEvent() {
        if(box != nullptr) {
            delete box;
        }
        box = nullptr;
    }
    virtual bool init() {return true;}

public:
    int what;
    netkit::IBox *box;
};


#endif //__MsgEvent_2014_10_14_H_
