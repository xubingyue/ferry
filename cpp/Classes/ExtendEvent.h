#ifndef __EVENTEXTEND_H__
#define __EVENTEXTEND_H__

#include <iostream>
#include <map>
#include "EventBus.h"
#include "IBox.h"
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

public:
    netkit::IBox *box;
    std::map<std::string, std::string> mapData;
};

#endif
