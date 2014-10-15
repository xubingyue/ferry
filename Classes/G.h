//
// Created by dantezhu on 14-10-15.
//


#ifndef __G_2014_10_15_H_
#define __G_2014_10_15_H_

#include <iostream>
#include "ferry.h"
#include "Box.h"
#include "EventBus.h"

class G
{
public:
    static ferry::Service<netkit::Box>* getFerry() {
        static ferry::Service<netkit::Box> ferry_;
        return &ferry_;
    }

    static eventbus::EventBus* getEventBus() {
        static eventbus::EventBus eventBus_;
        return &eventBus_;
    }


protected:
    G(){}
};



#endif //__G_2014_10_15_H_
