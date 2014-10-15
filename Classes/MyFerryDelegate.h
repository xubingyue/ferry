//
// Created by dantezhu on 14-10-15.
//


#ifndef __MyFerryDelegate_2014_10_15_H_
#define __MyFerryDelegate_2014_10_15_H_

#include "ferry.h"
#include "cocos2d.h"
#include "Box.h"
#include "ExtendEvent.h"


class MyFerryDelegate : public ferry::Delegate<netkit::Box> {

    virtual void onOpen(ferry::Service<netkit::Box> *service) {
        netkit::Box *box = new netkit::Box();
        box->cmd = 1;
        box->setBody("aini", 4);

        service->send(box);
    }

    virtual void onMessage(ferry::Service<netkit::Box> *service, netkit::Box *box) {
        cocos2d::log("[%s]-[%s][%d][%s] cmd: %d, sn: %d, ret: %d", FERRY_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                box->cmd, box->sn, box->ret);

        ExtendEvent* event = new ExtendEvent();
        event->what = 1;
        event->box = box;

        G::getEventBus()->pushEvent(event);
    }

    virtual void onClose(ferry::Service<netkit::Box> *service) {
        service->connect();
    }
};


#endif //__MyFerryDelegate_2014_10_15_H_
