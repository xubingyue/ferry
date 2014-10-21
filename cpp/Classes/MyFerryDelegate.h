//
// Created by dantezhu on 14-10-15.
//


#ifndef __MyFerryDelegate_2014_10_15_H_
#define __MyFerryDelegate_2014_10_15_H_

#include "ferry.h"
#include "cocos2d.h"
#include "Box.h"
#include "ExtendEvent.h"


#define FERRY_DELEGATE_LOG_TAG "ferry_delegate"


class MyFerryDelegate : public ferry::Delegate {

    virtual void onOpen(ferry::Service *service) {
        cocos2d::log("[%s]-[%s][%d][%s]", FERRY_DELEGATE_LOG_TAG, __FILE__, __LINE__, __FUNCTION__);

        netkit::Box *box = new netkit::Box();
        box->cmd = 1;
        box->setBody("aini", 4);

        service->send(box);
    }

    virtual void onSend(ferry::Service *service, netkit::IBox *ibox) {
        netkit::Box* box = (netkit::Box*)ibox;

        cocos2d::log("[%s]-[%s][%d][%s] cmd: %d, sn: %d, ret: %d", FERRY_DELEGATE_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                box->cmd, box->sn, box->ret);
    }

    virtual void onRecv(ferry::Service *service, netkit::IBox *ibox) {
        netkit::Box* box = (netkit::Box*)ibox;

        cocos2d::log("[%s]-[%s][%d][%s] cmd: %d, sn: %d, ret: %d", FERRY_DELEGATE_LOG_TAG, __FILE__, __LINE__, __FUNCTION__,
                box->cmd, box->sn, box->ret);

        ExtendEvent* event = new ExtendEvent();
        event->what = 1;
        event->box = box;

        G::getEventBus()->pushEvent(event);
    }

    virtual void onClose(ferry::Service *service) {
        cocos2d::log("[%s]-[%s][%d][%s]", FERRY_DELEGATE_LOG_TAG, __FILE__, __LINE__, __FUNCTION__);

        service->connect();
    }

    virtual void onError(ferry::Service *service, int code){
        cocos2d::log("[%s]-[%s][%d][%s] code: %d", FERRY_DELEGATE_LOG_TAG, __FILE__, __LINE__, __FUNCTION__, code);
    };

    virtual netkit::IBox* createBox() {
        return new netkit::Box();
    }
};


#endif //__MyFerryDelegate_2014_10_15_H_
