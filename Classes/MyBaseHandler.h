//
// Created by dantezhu on 14-10-15.
//


#ifndef __MyBaseHandler_2014_10_15_H_
#define __MyBaseHandler_2014_10_15_H_

#include "EventBus.h"
#include "G.h"


class MyBaseHandler: public eventbus::IHandler {
public:
    MyBaseHandler() {
        G::instance()->getEventBus()->regHandler(this);
    }

    ~MyBaseHandler() {
        G::instance()->getEventBus()->delHandler(this);
    }
};


#endif //__MyBaseHandler_2014_10_15_H_
