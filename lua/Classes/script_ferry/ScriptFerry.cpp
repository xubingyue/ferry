//
// Created by dantezhu on 14-10-23.
//

#include "ScriptFerry.h"
#include "ScriptBox.h"

namespace ferry {

ScriptFerry *ScriptFerry::getInstance() {
    static ScriptFerry *instance;
    if (!instance) {
        instance = new ScriptFerry();
    }
    return instance;
}

ScriptFerry::~ScriptFerry() {
    scriptDelAllCallbacks();
}

void ScriptFerry::scriptDelCallbacksForTarget(cocos2d::Ref* target) {
    scriptDelRspCallbacksForTarget(target);
    scriptDelEventCallbacksForTarget(target);
}

void ScriptFerry::scriptDelAllCallbacks() {
    scriptDelAllRspCallbacks();
    scriptDelAllEventCallbacks();
}

int ScriptFerry::scriptSend(netkit::IBox *box, int handler, float timeout, cocos2d::Ref *target) {
    int sn = newBoxSn();

    setSnToBox(box, sn);

    struct timeval tvNow, tvTimeout;

    // 当前时间
    gettimeofday(&tvNow, NULL);

    // 超时
    tvTimeout.tv_sec = (int)(timeout);
    tvTimeout.tv_usec = (int)((timeout - tvTimeout.tv_sec) * 1000000);

    ScriptRspCallbackContainer *container = new ScriptRspCallbackContainer();
    container->sn = sn;
    container->target = target;

    timeradd(&tvNow, &tvTimeout, &container->expireTime);
    container->scriptCallbackEntry = ScriptCallbackEntry::create(handler);
    // +1 一次，因为析构的时候会-1
    container->scriptCallbackEntry->retain();

    for (auto it = m_scriptRspCallbacks.begin();; it ++) {
        if (it == m_scriptRspCallbacks.end() ||
                timercmp(&(*it)->expireTime, &container->expireTime, >) ) {
            m_scriptRspCallbacks.insert(it, container);
            break;
        }
    }

    m_service.send(box);

    // 返回entryid，用户可以反注册
    // 当然，也可能那个时候已经超时
    return container->scriptCallbackEntry->getEntryId();
}

void ScriptFerry::scriptDelRspCallback(int entryID) {
    for(auto it = m_scriptRspCallbacks.begin(); it != m_scriptRspCallbacks.end();) {
        auto& container = *it;
        if (container->scriptCallbackEntry->getEntryId() == entryID) {
            cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());
            // 移除
            delete container;
            it = m_scriptRspCallbacks.erase(it);
        }
        else {
            ++it;
        }

    }
}

void ScriptFerry::scriptDelRspCallbacksForTarget(cocos2d::Ref *target) {
    for(auto it = m_scriptRspCallbacks.begin(); it != m_scriptRspCallbacks.end();) {
        auto& container = *it;
        if (container->target == target) {
            cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());
            // 移除
            delete container;
            it = m_scriptRspCallbacks.erase(it);
        }
        else {
            ++it;
        }

    }
}

void ScriptFerry::scriptDelAllRspCallbacks() {
    for (auto& container: m_scriptRspCallbacks) {
        cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());
        delete container;
    }

    m_scriptRspCallbacks.clear();
}

int ScriptFerry::scriptAddEventCallback(int handler, cocos2d::Ref *target) {

    ScriptEventCallbackContainer *container = new ScriptEventCallbackContainer();

    container->scriptCallbackEntry = ScriptCallbackEntry::create(handler);
    container->scriptCallbackEntry->retain();
    container->target = target;

    m_scriptEventCallbacks.push_back(container);

    return container->scriptCallbackEntry->getEntryId();
}

void ScriptFerry::scriptDelEventCallback(int entryID) {
    for(auto it = m_scriptEventCallbacks.begin(); it != m_scriptEventCallbacks.end();) {
        auto& container = *it;
        if (container->scriptCallbackEntry->getEntryId() == entryID) {
            cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());

            delete container;
            // 移除
            it = m_scriptEventCallbacks.erase(it);
        }
        else {
            ++it;
        }
    }

}

void ScriptFerry::scriptDelEventCallbacksForTarget(cocos2d::Ref *target) {
    for(auto it = m_scriptEventCallbacks.begin(); it != m_scriptEventCallbacks.end();) {
        auto& container = *it;
        if (container->target == target) {
            cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());

            delete container;
            // 移除
            it = m_scriptEventCallbacks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ScriptFerry::scriptDelAllEventCallbacks() {
    for (auto& container : m_scriptEventCallbacks) {
        cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());
        delete container;
    }

    m_scriptEventCallbacks.clear();
}


netkit::IBox* ScriptFerry::createBox() {
    return ScriptBox::create();
}

void ScriptFerry::onEvent(Event *event) {
    // 父类
    Ferry::onEvent(event);

    scriptOnEvent(event);
}
void ScriptFerry::checkRspTimeout() {
    // 父类
    Ferry::checkRspTimeout();

    scriptCheckRspTimeout();
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

void ScriptFerry::scriptCheckRspTimeout() {
    if (m_scriptRspCallbacks.empty()) {
        return;
    }

    struct timeval tvNow;
    gettimeofday(&tvNow, NULL);

    std::list<ScriptRspCallbackContainer*> todoCallbacks;

    // 提前申请好，免得每次都要传
    for(auto& container: m_scriptRspCallbacks) {
        if (timercmp(&tvNow, &container->expireTime, >)) {
            todoCallbacks.push_back(container);
        }
        else {
            break;
        }
    }

    if (todoCallbacks.empty()) {
        return;
    }

    // 再进入下一帧之前，不会释放
    ScriptEvent *scriptEvent = ScriptEvent::create();
    scriptEvent->what = EVENT_TIMEOUT;

    for(auto& container: todoCallbacks) {
        // 没找到
        if (std::find(m_scriptRspCallbacks.begin(), m_scriptRspCallbacks.end(), container) == m_scriptRspCallbacks.end()) {
            continue;
        }

        container->scriptCallbackEntry->call(scriptEvent);

        cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());

        delete container;

        m_scriptRspCallbacks.remove(container);
    }

}

void ScriptFerry::scriptHandleWithRspCallbacks(ScriptEvent *event) {
    auto scriptRspCallbacks = m_scriptRspCallbacks;
    int sn = getSnFromBox(event->box);

    for(auto it = scriptRspCallbacks.begin(); it != scriptRspCallbacks.end(); ++it) {
        auto& container = *it;
        if (container->sn == sn)
        {
            // 调用
            container->scriptCallbackEntry->call(event);
            // 移除
            if (event->what == EVENT_RECV || event->what == EVENT_ERROR) {
                cocos2d::ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(container->scriptCallbackEntry->getHandler());
                delete container;
                m_scriptRspCallbacks.remove(container);
            }
        }
    }
}

void ScriptFerry::scriptHandleWithEventCallbacks(ScriptEvent *event) {
    auto scriptEventCallbacks = m_scriptEventCallbacks;

    for(auto it = scriptEventCallbacks.begin(); it != scriptEventCallbacks.end(); ++it) {
        auto& container = *it;
        if (std::find(m_scriptEventCallbacks.begin(), m_scriptEventCallbacks.end(), container) == m_scriptEventCallbacks.end()) {
            // 已经中途被释放了
            continue;
        }

        container->scriptCallbackEntry->call(event);
    }
}

}
