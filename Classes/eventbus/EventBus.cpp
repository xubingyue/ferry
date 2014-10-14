#include "EventBus.h"

namespace eventbus {

    bool Event::init()
    {
        return true;
    }

    EventBus::EventBus():m_events(cocos2d::CCArray::create())
    {
        m_events->retain();
    }

    EventBus::~EventBus()
    {
        m_events->release();
    }

    void EventBus::reg(IHandler* handler)
    {
        m_handlers.insert(handler);
    }

    void EventBus::unreg(IHandler* handler)
    {
        m_handlers.erase(handler);
    }

    void EventBus::onEvent(Event* e)
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
                cout<<"Handler("<<*it<<"):Handler onEvent error"<<endl;
            }

            handlers.erase(*it);
        }
    }

    void EventBus::pushEvent(Event* event_)
    {
        m_events->addObject(event_);
    }

    void EventBus::loopEvents()
    {
        for(int i=0;i< m_events->count();++i){
            onEvent((Event*)m_events->getObjectAtIndex(i));
        }
        m_events->removeAllObjects();
    }

}
