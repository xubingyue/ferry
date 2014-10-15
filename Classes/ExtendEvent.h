#ifndef __EVENTEXTEND_H__
#define __EVENTEXTEND_H__

#include <iostream>
#include <map>
#include "EventBus.h"
#include "Box.h"
#include "cocos2d.h"

class ExtendEvent :public eventbus::BaseEvent{
public:
    ExtendEvent() {
        box = nullptr;
    }
    virtual ~ExtendEvent() {
        if(box) {
            delete box;
        }
        box = nullptr;
    }
    virtual bool init() {return true;}

public:
    netkit::Box *box;
};

#endif