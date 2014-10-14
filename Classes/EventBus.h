#ifndef __EVENTBUS_H__
#define __EVENTBUS_H__

#include <iostream>
#include <set>
#include "cocos2d.h"

using namespace std;
using namespace cocos2d;
namespace eventbus{
	class Event:public cocos2d::Ref
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
		virtual void onEvent(Event* )=0;
	};

	class EventBus
	{
	public:
		EventBus();
		~EventBus();
		void Register(IHandler* );
		void UnRegister(IHandler* );
		void pushEvent(Event* );
		void loopEvents();
	private:
		void OnEvent(Event* e);
	private:
		cocos2d::CCArray* m_events;
		set<IHandler*> m_handler;
	};

}

#endif //EventBus.h