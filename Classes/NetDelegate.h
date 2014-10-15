//
// Created by dantezhu on 14-10-15.
//


#ifndef __NetDelegate_2014_10_15_H_
#define __NetDelegate_2014_10_15_H_

#include "ferry/Delegate.h"
#include "cocos2d.h"


template <class T>
class NetDelegate: public ferry::Delegate<T> {

    virtual void onOpen(ferry::NetService<T> *net) {
        cocos2d::log("[%s]-[%s][%d][%s] null delegate", "app_log", __FILE__, __LINE__, __FUNCTION__);
    }
    virtual void onMessage(ferry::NetService<T> *net, T *box) {
        cocos2d::log("[%s]-[%s][%d][%s] null delegate", "app_log", __FILE__, __LINE__, __FUNCTION__);
    }
    virtual void onClose(ferry::NetService<T> *net) {
        cocos2d::log("[%s]-[%s][%d][%s] null delegate", "app_log", __FILE__, __LINE__, __FUNCTION__);
    }
};


#endif //__NetDelegate_2014_10_15_H_
