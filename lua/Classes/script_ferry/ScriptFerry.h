//
// Created by dantezhu on 14-10-23.
//


#ifndef __FerryScript_2014_10_23_H_
#define __FerryScript_2014_10_23_H_

#include "cocos2d.h"
#include "ferry/Ferry.h"
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
    void reset() {
        sn = 0;
        m_scriptCallbackEntry = 0;
    }

    ScriptRspCallbackContainer() {
        reset();
    }

    ~ScriptRspCallbackContainer() {
        if (m_scriptCallbackEntry) {
            m_scriptCallbackEntry->release();
        }
    }

    ScriptRspCallbackContainer(const ScriptRspCallbackContainer & container) {
        // list insert 用的是copy构造函数
        reset();

        sn = container.sn;
        expireTime = container.expireTime;

        // 这样会自动retain
        setScriptCallbackEntry(container.getScriptCallbackEntry());
    }

    ScriptRspCallbackContainer& operator = (const ScriptRspCallbackContainer& container) {
        // 这里不能调用reset，因为这个时候对象的数据已经是真实的

        // map 中用的是赋值运算符
        sn = container.sn;
        expireTime = container.expireTime;

        // 这样会自动retain
        setScriptCallbackEntry(container.getScriptCallbackEntry());
        return *this;
    }

    int sn;
    struct timeval expireTime;

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
    void scriptDelRspCallback(int entryID);
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
    std::list<ScriptRspCallbackContainer> m_scriptListRspCallbacks;

    cocos2d::Map<int, ScriptCallbackEntry*> m_scriptMapEventCallbacks;
};


}
#endif //__FerryScript_2014_10_23_H_
