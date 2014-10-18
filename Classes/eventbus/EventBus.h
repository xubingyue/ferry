#ifndef __EVENTBUS_H_20141014190353__
#define __EVENTBUS_H_20141014190353__

#include <iostream>
#include <set>
#include <map>
#include <list>
#include <pthread.h>

namespace eventbus {
    class BaseEvent
    {
    public:
        BaseEvent() {
            what = 0;
			m_done = false;
        }
        // 如果不标明虚函数，进行继承后delete会有bug
        virtual ~BaseEvent() {}

        bool isDone() {
            return m_done;
        }

        // 应用层不许调用
        void setDone(bool done) {
            m_done = done;
        }

        int what;
        std::map<std::string, std::string> mapData;

    private:
		// 是否完成
		bool m_done;
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
        virtual ~EventBus();
        void addHandler(IHandler *);
        void delHandler(IHandler *);
        void pushEvent(BaseEvent*);
        void loopEvents();
    private:
        void onEvent(BaseEvent* e);
        void clearEvents();
    private:
        std::list<BaseEvent*> m_events;
        std::set<IHandler*> m_handlers;

		pthread_mutex_t m_visit_mutex;
    };

}

#endif /* end of include guard: __EVENTBUS_H_20141014190353__ */
