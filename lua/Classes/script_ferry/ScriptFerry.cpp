//
// Created by dantezhu on 14-10-23.
//

#include "ScriptFerry.h"
#include "ScriptBox.h"

namespace ferry {

ScriptFerry *ScriptFerry::getInstance() {
    static ScriptFerry instance;
    return &instance;
}

void ScriptFerry::scriptDelAllCallbacks() {
    scriptDelAllRspCallbacks();
    scriptDelAllEventCallbacks();
}

int ScriptFerry::scriptSend(netkit::IBox *box, int handler, float timeout) {
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

void ScriptFerry::scriptDelAllRspCallbacks() {
    m_scriptMapRspCallbacks.clear();
}

void ScriptFerry::scriptDelAllEventCallbacks() {
    m_scriptMapEventCallbacks.clear();
}

int ScriptFerry::scriptAddEventCallback(int handler) {
    ScriptCallbackEntry* entry = ScriptCallbackEntry::create(handler);

    int entryID = entry->getEntryId();

    m_scriptMapEventCallbacks.insert(entryID, entry);

    return entryID;
}
void ScriptFerry::scriptDelEventCallback(int entryID) {
    m_scriptMapEventCallbacks.erase(entryID);
}

netkit::IBox* ScriptFerry::createBox() {
    return ScriptBox::create();
}

void ScriptFerry::onEvent(Event *event) {
    // 父类
    Ferry::onEvent(event);

    scriptOnEvent(event);
}
void ScriptFerry::onCheckRspTimeout() {
    // 父类
    Ferry::onCheckRspTimeout();

    scriptOnCheckRspTimeout();
}

void ScriptFerry::scriptOnEvent(Event *event) {
    ScriptEvent* scriptEvent = ScriptEvent::create();

    scriptEvent->what = event->what;
    scriptEvent->box = event->box;
    scriptEvent->code = event->code;

    if(scriptEvent->what == EVENT_RECV) {
        scriptHandleWithRspCallbacks(scriptEvent);
    }

    scriptHandleWithEventCallbacks(scriptEvent);
}

void ScriptFerry::scriptOnCheckRspTimeout() {
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

void ScriptFerry::scriptHandleWithRspCallbacks(ScriptEvent *event) {
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

void ScriptFerry::scriptHandleWithEventCallbacks(ScriptEvent *event) {
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