#ifndef __EVENTEXTEND_H__
#define __EVENTEXTEND_H__

#include <iostream>
#include <map>
#include "EventBus.h"
#include "Box.h"

class ExtendEvent :public eventbus::BaseEvent{
public:
    CREATE_FUNC(ExtendEvent);

    ExtendEvent() {
        box = nullptr;
        what = 0;
    }
    virtual ~ExtendEvent() {
        if(box != nullptr) {
            delete box;
        }
        box = nullptr;
    }
    virtual bool init() {return true;}

public:
    netkit::Box *box;
    int what;
    std::map <std::string, std::string> mapData;
};

#endif