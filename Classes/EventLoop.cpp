//
//  EventLoop.cpp
//  SlotsGame
//
//  Created by TUSM on 14-8-1.
//
//

#include "EventLoop.h"

USING_NS_CC;
bool EventLoop::init(){
    bool bRet = false;
    do{
        //dosomething
        log("Eventloop   init");
        //this->scheduleUpdate();
        
        bRet = true;
    }while (0);
    return bRet;
}


 void  EventLoop::loop(float dt){
    slotSingle = SlotWebSingle::getInstance();
    slotSingle->getEventBus()->loopEvents();
}
