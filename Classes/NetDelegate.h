//
// Created by dantezhu on 14-10-15.
//


#ifndef __NetDelegate_2014_10_15_H_
#define __NetDelegate_2014_10_15_H_

#include "ferry/Delegate.h"
#include "cocos2d.h"
#include "Box.h"


class NetDelegate: public ferry::Delegate<netkit::Box> {

    virtual void onOpen(ferry::NetService<netkit::Box> *netService) {
        cocos2d::log("[%s]-[%s][%d][%s]", "app_log", __FILE__, __LINE__, __FUNCTION__);
    }
    virtual void onMessage(ferry::NetService<netkit::Box> *netService, netkit::Box *box) {
        cocos2d::log("[%s]-[%s][%d][%s]", "app_log", __FILE__, __LINE__, __FUNCTION__);
    }
    virtual void onClose(ferry::NetService<netkit::Box> *netService) {
        cocos2d::log("[%s]-[%s][%d][%s]", "app_log", __FILE__, __LINE__, __FUNCTION__);
        netService->connect();
    }
};


#endif //__NetDelegate_2014_10_15_H_
