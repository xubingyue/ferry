#ifndef __EVENTEXTEND_H__
#define __EVENTEXTEND_H__

#include "EventBus.h"
#include "Box.h"

class ExtendEvent :public eventbus::BaseEvent{
public:
    CREATE_FUNC(ExtendEvent);

    ExtendEvent();
    virtual ~ExtendEvent() {
        if(box != nullptr) {
            delete box;
        }
        box = nullptr;
    }
    virtual bool init();

public:
    netkit::Box  *box;
    int what;
    map <string,string> mapData;
};

#endif