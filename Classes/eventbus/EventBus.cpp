#include "EventBus.h"

namespace eventbus {

    EventBus::EventBus()
    {
    }

    EventBus::~EventBus()
    {
        m_events.clear();
    }

    void EventBus::reg(IHandler* handler)
    {
        m_handlers.insert(handler);
    }

    void EventBus::unReg(IHandler *handler)
    {
        m_handlers.erase(handler);
    }

    void EventBus::onEvent(BaseEvent* e)
    {
        set<IHandler*> handlers=m_handlers;

        while(1) {
            set<IHandler*> tmp_handler;

            set_intersection(handlers.begin(), handlers.end(),
                    m_handlers.begin(), m_handlers.end(),
                    insert_iterator<set<IHandler*> >(tmp_handler, tmp_handler.begin()));

            if (tmp_handler.empty()) {
                break;
            }

            auto it = tmp_handler.begin();
            try{
                (*it)->onEvent(e);
            }
            catch(...)
            {
                cocos2d::log("handler event exc");
            }

            handlers.erase(*it);
        }
    }

    void EventBus::pushEvent(BaseEvent* event)
    {
        m_events.pushBack(event);
    }

    void EventBus::loopEvents()
    {
        for (auto& e: m_events) {
            onEvent(e);
        }

        m_events.clear();
    }

}
