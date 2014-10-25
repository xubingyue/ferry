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
    callbackContainer.sn = sn;
    // timeout 不强制转int会有问题
    callbackContainer.expireTime = time(NULL) + (int)timeout;
    // 会自动retain
    callbackContainer.setScriptCallbackEntry(ScriptCallbackEntry::create(handler));

    for (auto it = m_scriptListRspCallbacks.begin();; it ++) {
        if (it == m_scriptListRspCallbacks.end() ||
                it->expireTime > callbackContainer.expireTime) {
            m_scriptListRspCallbacks.insert(it, callbackContainer);
            break;
        }
    }

    m_service.send(box);

    // 返回entryid，用户可以反注册
    // 当然，也可能那个时候已经超时
    return callbackContainer.getScriptCallbackEntry()->getEntryId();
}

void ScriptFerry::scriptDelRspCallback(int entryID) {
    for(auto it = m_scriptListRspCallbacks.begin(); it != m_scriptListRspCallbacks.end();) {
        auto& container = *it;
        auto tempit = it;
        it++;
        if (container.getScriptCallbackEntry()->getEntryId() == entryID)
        {
            cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container.getScriptCallbackEntry()->getHandler());
            // 移除
            m_scriptListRspCallbacks.erase(tempit);
        }

    }
}

void ScriptFerry::scriptDelAllRspCallbacks() {
    m_scriptListRspCallbacks.clear();
}

void ScriptFerry::scriptDelAllEventCallbacks() {
    for(auto it=m_scriptMapEventCallbacks.begin(); it != m_scriptMapEventCallbacks.end(); it++) {
        cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(it->second->getHandler());
    }

    m_scriptMapEventCallbacks.clear();
}

int ScriptFerry::scriptAddEventCallback(int handler) {
    ScriptCallbackEntry* entry = ScriptCallbackEntry::create(handler);

    int entryID = entry->getEntryId();

    m_scriptMapEventCallbacks.insert(entryID, entry);

    return entryID;
}
void ScriptFerry::scriptDelEventCallback(int entryID) {
    if (m_scriptMapEventCallbacks.find(entryID) == m_scriptMapEventCallbacks.end()) {
        return;
    }

    auto entry = m_scriptMapEventCallbacks.at(entryID);
    cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(entry->getHandler());

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

    if (scriptEvent->box && getSnFromBox(scriptEvent->box) > 0) {
        scriptHandleWithRspCallbacks(scriptEvent);
    }
    else {
        scriptHandleWithEventCallbacks(scriptEvent);
    }
}

void ScriptFerry::scriptOnCheckRspTimeout() {
    time_t nowTime = time(NULL);

    // 再进入下一帧之前，不会释放
    ScriptEvent *scriptEvent = ScriptEvent::create();
    scriptEvent->what = EVENT_TIMEOUT;

    // 提前申请好，免得每次都要传
    for(auto it = m_scriptListRspCallbacks.begin(); it != m_scriptListRspCallbacks.end();) {
        auto& container = *it;
        auto tempit = it;
        it++;
        if (nowTime >= container.expireTime)
        {
            // 超时了
            container.getScriptCallbackEntry()->call(scriptEvent);

            cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container.getScriptCallbackEntry()->getHandler());

            // 移除
            m_scriptListRspCallbacks.erase(tempit);
        }
        else {
            break;
        }
    }
}

void ScriptFerry::scriptHandleWithRspCallbacks(ScriptEvent *event) {
    int sn = getSnFromBox(event->box);

    for(auto it = m_scriptListRspCallbacks.begin(); it != m_scriptListRspCallbacks.end();) {
        auto& container = *it;
        auto tempit = it;
        it++;
        if (container.sn == sn)
        {
            // 调用
            container.getScriptCallbackEntry()->call(event);
            // 移除
            if (event->what == EVENT_RECV || event->what == EVENT_ERROR) {
                cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container.getScriptCallbackEntry()->getHandler());
                m_scriptListRspCallbacks.erase(tempit);
            }
        }
    }
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
