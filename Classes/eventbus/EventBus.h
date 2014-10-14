#ifndef __EVENTBUS_H_20141014190353__
#define __EVENTBUS_H_20141014190353__

#include <iostream>
#include <set>
#include "cocos2d.h"

using namespace std;

namespace eventbus{

    class BaseEvent: public cocos2d::Ref
    {
    public:
        BaseEvent(){}
        virtual ~BaseEvent(){}

        virtual bool init() {return true;}
        CREATE_FUNC(BaseEvent);
    };

    class IHandler
    {
    public:
        virtual ~IHandler(){}
        virtual void onEvent(BaseEvent*)=0;
    };

    class EventBus
    {
    public:
        EventBus();
        ~EventBus();
        void reg(IHandler*);
        void unReg(IHandler *);
        void pushEvent(BaseEvent*);
        void loopEvents();
    private:
        void onEvent(BaseEvent* e);
    private:
        cocos2d::Vector<BaseEvent*> m_events;
        set<IHandler*> m_handlers;
    };

}

#endif /* end of include guard: __EVENTBUS_H_20141014190353__ */
