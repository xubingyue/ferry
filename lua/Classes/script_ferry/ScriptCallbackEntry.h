//
// Created by dantezhu on 14-10-23.
//


#ifndef __FerryScriptSupport_2014_10_23_H_
#define __FerryScriptSupport_2014_10_23_H_

#include "CCScriptSupport.h"

namespace ferry {

class ScriptEvent;

class ScriptCallbackEntry : public cocos2d::ScriptHandlerEntry {
public:
    static ScriptCallbackEntry * create(int handler);

    ScriptCallbackEntry(int handler);
    virtual ~ScriptCallbackEntry(){}

    void call(ScriptEvent* event);
};

}

#endif //__FerryScriptSupport_2014_10_23_H_
