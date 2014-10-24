//
// Created by dantezhu on 14-10-23.
//

#include "FerryScript.h"

namespace ferry {

FerryScript* FerryScript::getInstance() {
    static FerryScript instance;
    return &instance;
}

void FerryScript::scriptDelAllCallbacks() {
    scriptDelAllRspCallbacks();
    scriptDelAllEventCallbacks();
}

int FerryScript::scriptSend(netkit::IBox *box, int handler, float timeout) {
    int sn = newBoxSn();

    setSnToBox(box, sn);

    ScriptRspCallbackContainer callbackContainer;
    callbackContainer.timeout = timeout;
    callbackContainer.createTime = time(NULL);
    // 会自动retain
    callbackContainer.setScriptCallbackEntry(ScriptCallbackEntry::create(handler));

    m_scriptMapRspCallbacks[sn] = callbackContainer;

    return m_service.send(box);
}

void FerryScript::scriptDelAllRspCallbacks() {
    m_scriptMapRspCallbacks.clear();
}

void FerryScript::scriptDelAllEventCallbacks() {
    m_scriptMapEventCallbacks.clear();
}

int FerryScript::scriptAddEventCallback(int handler) {
    ScriptCallbackEntry* entry = ScriptCallbackEntry::create(handler);

    int entryID = entry->getEntryId();

    m_scriptMapEventCallbacks.insert(entryID, entry);

    return entryID;
}
void FerryScript::scriptDelEventCallback(int entryID) {
    m_scriptMapEventCallbacks.erase(entryID);
}

void FerryScript::onEvent(Event *event) {
    // 父类
    Ferry::onEvent(event);

    scriptOnEvent(event);
}
void FerryScript::onCheckRspTimeout() {
    // 父类
    Ferry::onCheckRspTimeout();

    scriptOnCheckRspTimeout();
}

void FerryScript::scriptOnEvent(Event *event) {
    ScriptEvent* scriptEvent = ScriptEvent::create();

    scriptEvent->what = event->what;
    scriptEvent->box = event->box;
    scriptEvent->code = event->code;

    if(scriptEvent->what == EVENT_RECV) {
        scriptHandleWithRspCallbacks(scriptEvent);
    }

    scriptHandleWithEventCallbacks(scriptEvent);
}

void FerryScript::scriptOnCheckRspTimeout() {
    time_t nowTime = time(NULL);

    // 再进入下一帧之前，不会释放
    ScriptEvent *scriptEvent = ScriptEvent::create();
    scriptEvent->what = EVENT_TIMEOUT;

    // 提前申请好，免得每次都要传
    for(auto it = m_scriptMapRspCallbacks.begin(); it != m_scriptMapRspCallbacks.end();) {
        auto& container = it->second;
        float past = nowTime - container.createTime;
        auto tempit = it;
        it++;
        if (past > container.timeout)
        {
            // 超时了
            container.getScriptCallbackEntry()->call(scriptEvent);

            // 移除
            m_scriptMapRspCallbacks.erase(tempit);
        }
    }
}

void FerryScript::scriptHandleWithRspCallbacks(ScriptEvent *event) {
    int sn = getSnFromBox(event->box);

    if (m_scriptMapRspCallbacks.find(sn) == m_scriptMapRspCallbacks.end()) {
        // 没有找到
        return;
    }

    auto& container = m_scriptMapRspCallbacks[sn];
    if (container.getScriptCallbackEntry()) {
        container.getScriptCallbackEntry()->call(event);
    }

    m_scriptMapRspCallbacks.erase(sn);

}

void FerryScript::scriptHandleWithEventCallbacks(ScriptEvent *event) {
    auto mapScriptEventCallbacks = m_scriptMapEventCallbacks;

    for(auto it = mapScriptEventCallbacks.begin(); it != mapScriptEventCallbacks.end(); ++it) {
        auto entryID = it->first;

        if (m_scriptMapEventCallbacks.find(entryID) == m_scriptMapEventCallbacks.end()) {
            // 不用这个了
            continue;
        }

        it->second->call(event);
    }
}

}