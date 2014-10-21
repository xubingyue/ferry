//
// Created by dantezhu on 14-10-15.
//


#ifndef __MyBaseHandler_2014_10_15_H_
#define __MyBaseHandler_2014_10_15_H_

#include "EventBus.h"
#include "G.h"


class BaseEventHandler : public eventbus::IHandler {
public:
    BaseEventHandler() {
        G::getEventBus()->addHandler(this);
    }

    ~BaseEventHandler() {
        G::getEventBus()->delHandler(this);
    }
};


#endif //__MyBaseHandler_2014_10_15_H_
