﻿#ifndef __EVENTBUS_H_20141014190353__
#define __EVENTBUS_H_20141014190353__

#include <iostream>
#include <set>
#include <map>
#include <list>

namespace eventbus{

    class BaseEvent
    {
    public:
        int what;
        std::map<std::string, std::string> mapData;
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
        void regHandler(IHandler *);
        void delHandler(IHandler *);
        void pushEvent(BaseEvent*);
        void loopEvents();
    private:
        void onEvent(BaseEvent* e);
    private:
        std::list<BaseEvent*> m_events;
        std::set<IHandler*> m_handlers;
    };

}

#endif /* end of include guard: __EVENTBUS_H_20141014190353__ */
