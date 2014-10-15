//
// Created by dantezhu on 14-10-15.
//


#ifndef __NetDelegate_2014_10_15_H_
#define __NetDelegate_2014_10_15_H_

#include "ferry.h"
#include "cocos2d.h"
#include "Box.h"


class MyFerryDelegate : public ferry::Delegate<netkit::Box> {

    virtual void onOpen(ferry::Service<netkit::Box> *service) {
    }
    virtual void onMessage(ferry::Service<netkit::Box> *service, netkit::Box *box) {
    }
    virtual void onClose(ferry::Service<netkit::Box> *service) {
        service->connect();
    }
};


#endif //__NetDelegate_2014_10_15_H_
