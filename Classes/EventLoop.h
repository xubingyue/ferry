//
//  EventLoop.h
//  SlotsGame
//
//  Created by TUSM on 14-8-1.
//
//

#ifndef SlotsGame_EventLoop_h
#define SlotsGame_EventLoop_h
#include "cocos2d.h"
#include "SlotWebSingle.h"
USING_NS_CC;

class EventLoop:public Node{
public:
    CREATE_FUNC(EventLoop);
    bool init();
    void loop(float dt);
private:
	EventLoop();
    SlotWebSingle* slotSingle;
};

#endif
