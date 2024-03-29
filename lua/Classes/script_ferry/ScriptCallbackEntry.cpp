//
// Created by dantezhu on 14-10-23.
//

#include "ScriptCallbackEntry.h"
#include "CCLuaEngine.h"
#include "ScriptFerry.h"

namespace ferry {
ScriptCallbackEntry *ScriptCallbackEntry::create(int handler) {

    ScriptCallbackEntry* entry = new ScriptCallbackEntry(handler);
    entry->autorelease();
    return entry;
}

ScriptCallbackEntry::ScriptCallbackEntry(int handler):ScriptHandlerEntry(handler) {
}

void ScriptCallbackEntry::call(ScriptEvent* event) {
    cocos2d::LuaStack* stack = cocos2d::LuaEngine::getInstance()->getLuaStack();
    stack->pushObject(event, "ferry.ScriptEvent");
    // 1 代表1个参数
    int ret = stack->executeFunctionByHandler(_handler, 1);
    stack->clean();
}

}