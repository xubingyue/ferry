#ifndef __EVENTBUS_H_20141014190353__
#define __EVENTBUS_H_20141014190353__

#include <iostream>
#include <set>
#include "cocos2d.h"

using namespace std;

namespace eventbus{
    class Event: public cocos2d::Ref
    {
    public:
        Event(){}
        virtual ~Event(){}

        virtual bool init();
        CREATE_FUNC(Event);
    };

    class IHandler
    {
    public:
        virtual ~IHandler(){}
        virtual void onEvent(Event*)=0;
    };

    class EventBus
    {
    public:
        EventBus();
        ~EventBus();
        void reg(IHandler*);
        void unreg(IHandler*);
        void pushEvent(Event*);
        void loopEvents();
    private:
        void onEvent(Event* e);
    private:
        cocos2d::CCArray* m_events;
        //cocos2d::Vector<IHandler*>* m_events2;
        set<IHandler*> m_handlers;
    };

}

#endif /* end of include guard: __EVENTBUS_H_20141014190353__ */
