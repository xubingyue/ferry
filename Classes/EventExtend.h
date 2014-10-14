#ifndef __EVENTEXTEND_H__
#define __EVENTEXTEND_H__

#include "EventBus.h"
#include "Box.h"

using namespace netkit;


class  EventExtend:public eventbus::Event{
public:
	EventExtend();
	//what are you doing box is autorelease object why delete it manually 
	virtual ~EventExtend(){if(box != nullptr)delete box; box = nullptr;}

	virtual bool init();

	Box  *box;

	int what;

	map <string,string> map_act; 

	CREATE_FUNC(EventExtend);
    

};

#endif