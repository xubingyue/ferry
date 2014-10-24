//
// Created by dantezhu on 14-10-23.
//


#ifndef __FerryScript_2014_10_23_H_
#define __FerryScript_2014_10_23_H_

#include "cocos2d.h"
#include "Ferry.h"
#include "ScriptCallbackEntry.h"


namespace ferry {

class ScriptEvent: public cocos2d::Ref {
public:
    ScriptEvent() {
        what = 0;
        box = nullptr;
        code = 0;
    }
    virtual ~ScriptEvent() {}

    CREATE_FUNC(ScriptEvent);

    bool init() {return true;}

    int getWhat() {
        return what;
    }

    netkit::IBox* getBox() {
        return box;
    }

    int getCode() {
        return code;
    }

    int what;
    netkit::IBox *box;
    int code;
};

class ScriptRspCallbackContainer {
public:
    ScriptRspCallbackContainer() {
        m_scriptCallbackEntry = nullptr;
    }

    ~ScriptRspCallbackContainer() {
        if (m_scriptCallbackEntry) {
            m_scriptCallbackEntry->release();
        }
    }

    ScriptRspCallbackContainer(const ScriptRspCallbackContainer & container) {
        createTime = container.createTime;
        timeout = container.timeout;
        callback = container.callback;
        target = container.target;

        // 这样会自动retain
        setScriptCallbackEntry(container.getScriptCallbackEntry());
    }

    ScriptRspCallbackContainer& operator = (const ScriptRspCallbackContainer& container) {
        // 有用到了赋值运算
        // map 中用的居然是赋值运算符
        createTime = container.createTime;
        timeout = container.timeout;
        callback = container.callback;
        target = container.target;

        // 这样会自动retain
        setScriptCallbackEntry(container.getScriptCallbackEntry());
        return *this;
    }

    time_t createTime;
    float timeout;
    CallbackType callback;
    void* target;


    ScriptCallbackEntry* getScriptCallbackEntry () const{
        return m_scriptCallbackEntry;
    }

    void setScriptCallbackEntry(ScriptCallbackEntry *scriptCallbackEntry) {
        // 先retain、再release，防止是同一个对象
        if (scriptCallbackEntry) {
            scriptCallbackEntry->retain();
        }
        if (m_scriptCallbackEntry) {
            m_scriptCallbackEntry->release();
        }

        m_scriptCallbackEntry = scriptCallbackEntry;
    }
private:
    ScriptCallbackEntry *m_scriptCallbackEntry;
};


class ScriptFerry : public Ferry {
public:
    static ScriptFerry * getInstance();

    void scriptDelAllCallbacks();

    int scriptSend(netkit::IBox *box, int handler, float timeout);
    void scriptDelAllRspCallbacks();

    int scriptAddEventCallback(int handler);
    void scriptDelEventCallback(int entryID);
    void scriptDelAllEventCallbacks();

    virtual netkit::IBox *createBox();

protected:
    virtual void onEvent(Event *event);
    virtual void onCheckRspTimeout();

    void scriptOnEvent(Event *event);
    void scriptOnCheckRspTimeout();

    void scriptHandleWithRspCallbacks(ScriptEvent *event);
    void scriptHandleWithEventCallbacks(ScriptEvent *event);

protected:
    std::map<int, ScriptRspCallbackContainer> m_scriptMapRspCallbacks;

    cocos2d::Map<int, ScriptCallbackEntry*> m_scriptMapEventCallbacks;
};


}
#endif //__FerryScript_2014_10_23_H_
