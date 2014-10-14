/*
*/

#include "EventBus.h" 
#include "Defines.h"
#include "EventExtend.h"
#include "SlotWebSingle.h"
//EventBus functions

/*init the m_events*/
eventbus::EventBus::EventBus():m_events(cocos2d::CCArray::create())
{
	m_events->retain();
}

eventbus::EventBus::~EventBus()
{
	m_events->release();
}

void eventbus::EventBus::Register(IHandler* hand)
{
	m_handler.insert(hand);
}

void eventbus::EventBus::UnRegister(IHandler* hand)
{
	m_handler.erase(hand);
}

void eventbus::EventBus::OnEvent(Event* e)
{
	set<IHandler*> handler2=m_handler;

	while(1) {
		set<IHandler*> tmp_handler;

		set_intersection(handler2.begin(), handler2.end(), m_handler.begin(), m_handler.end(), insert_iterator<set<IHandler*> >(tmp_handler, tmp_handler.begin()));

		if (tmp_handler.empty()) {
			break;
		}

		auto it = tmp_handler.begin();
		try{
			(*it)->onEvent(e);
		}
		catch(...)
		{
			cout<<"Handler("<<*it<<"):Handler OnEvent error"<<endl;
		}

		handler2.erase(*it);
	}
}

void eventbus::EventBus::pushEvent(Event* event_)
{
	m_events->addObject(event_);
}

void eventbus::EventBus::loopEvents()
{
	auto web = SlotWebSingle::getInstance();
	for(int i=0;i< m_events->count();++i){
		auto what = ((EventExtend*)(m_events->getObjectAtIndex(i)))->what;
		if ( what== EVENT_DISCONNECT_CONNECT)
		{
			#if ENABLE_LOG==1
			log("EventBus : %d : loopEvents() : disconnect !!",__LINE__);
			#endif
			if (web->getWebSocket()->isConnected() == false)
			{
				web->connectToServer();
				#if ENABLE_LOG==1
				log("EventBus : %d : loopEvents() : reconnect to server !!", __LINE__);
				#endif
			}
		}
		else if (what == EVENT_CONNECT_NET)
		{
			#if ENABLE_LOG==1
			log("EventBus : %d : loopEvents() : connect to server succeed !!", __LINE__);
			#endif

		}
		
		OnEvent((Event*)m_events->getObjectAtIndex(i));

	}
	m_events->removeAllObjects();
}

//Event

bool eventbus::Event::init()
{
	return true;
}

//EventBus.cpp