#ifndef __EVENTBUS_H_20141014190353__
#define __EVENTBUS_H_20141014190353__

#include <iostream>
#include <set>
#include <map>
#include <list>
#include <pthread.h>

namespace eventbus{

    class BaseEvent
    {
    public:
        BaseEvent() {
            what = 0;
			_handled = false;
        }
        // 如果不标明虚函数，进行继承后delete会有bug
        virtual ~BaseEvent() {}
        int what;
        std::map<std::string, std::string> mapData;

		// 是否已经处理过. 外面不要使用
		bool _handled;
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
