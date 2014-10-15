#include "EventBus.h"

namespace eventbus {

    EventBus::EventBus()
    {
    }

    EventBus::~EventBus()
    {
        clearEvents();
        m_handlers.clear();
    }

    void EventBus::regHandler(IHandler* handler)
    {
        m_handlers.insert(handler);
    }

    void EventBus::delHandler(IHandler *handler)
    {
        m_handlers.erase(handler);
    }

    void EventBus::onEvent(BaseEvent* e)
    {
        std::set<IHandler*> handlers=m_handlers;

        while(1) {
            std::set<IHandler*> tmp_handler;

            std::set_intersection(handlers.begin(), handlers.end(),
                    m_handlers.begin(), m_handlers.end(),
                    std::insert_iterator<std::set<IHandler*> >(tmp_handler, tmp_handler.begin()));

            if (tmp_handler.empty()) {
                break;
            }

            auto it = tmp_handler.begin();
            try{
                (*it)->onEvent(e);
            }
            catch(...)
            {
            }

            handlers.erase(*it);
        }
    }

    void EventBus::pushEvent(BaseEvent* event)
    {
        m_events.push_back(event);
    }

    void EventBus::loopEvents()
    {
        for (auto& e: m_events) {
            onEvent(e);
            // event在用完了之后就要删掉
            delete e;
        }

        m_events.clear();
    }

    void EventBus::clearEvents() {
        for (auto& e: m_events) {
            delete e;
        }
        m_events.clear();
    }

}
