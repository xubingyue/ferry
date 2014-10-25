//
// Created by dantezhu on 14-10-22.
//


#ifndef __Ferry_2014_10_22_H_
#define __Ferry_2014_10_22_H_

#include <iostream>
#include <map>
#include <set>
#include <list>
#include <functional>
#include <pthread.h>

// 如果cocos2d.h不放在最前，win下会编译报错
#include "cocos2d.h"
#include "Delegate.h"
#include "Service.h"
#include "Constants.h"
#include "Utils.h"

namespace ferry {

class Event {
public:
    Event() {
        what = 0;
        box = nullptr;
        code = 0;
        _done = false;
    }

    virtual ~Event() {
        if(box) {
            delete box;
        }
        box = nullptr;
    }

public:
    int what;
    netkit::IBox *box;
    int code;

    bool _done;
};

// 事件注册的回调
typedef std::function<void(Event*)> CallbackType;

struct RspCallbackContainer {
    int sn;
    struct timeval expireTime;
    CallbackType callback;
    void* target;
};

class Ferry : public ferry::Delegate {
public:
    static Ferry *getInstance();

    Ferry();

    virtual ~Ferry();

    ferry::Service* getService();

    virtual int init(const std::string &host, short port);

    void start();

    void stop();

    void connect();

    // 删除类对应的所有回调，务必在使用ferry的类的析构函数里调用
    void delCallbacksForTarget(void *target);
    // 删除所有回调
    void delAllCallbacks();

    // 发送消息
    void send(netkit::IBox *box);
    // 带回调的发送，以及超时，超时为秒。target很有用，可以用来防止崩溃
    // callback 会收到 SEND, RECV, ERROR, TIMEOUT 事件
    void send(netkit::IBox *box, CallbackType callback, float timeout, void* target);

    // 删除send对应的回调
    void delRspCallbacksForTarget(void *target);
    void delAllRspCallbacks();

    // 注册事件回调
    // callback 会收到 OPEN, CLOSE, SEND, RECV, ERROR, TIMEOUT 事件
    // 派给RspCallback的请求，不会再EventCallback中收到
    void addEventCallback(CallbackType callback, void* target, const std::string& name);
    void delEventCallback(const std::string& name, void* target);
    void delEventCallbacksForTarget(void *target);
    void delAllEventCallbacks();

    // Delegate begin
    virtual void onOpen(ferry::Service *service);

    virtual void onSend(ferry::Service *service, netkit::IBox *ibox);

    virtual void onRecv(ferry::Service *service, netkit::IBox *ibox);

    virtual void onClose(ferry::Service *service);

    virtual void onError(ferry::Service *service, int code, netkit::IBox *ibox);

    virtual netkit::IBox *createBox();
    // Delegate end

    virtual void setSnToBox(netkit::IBox* ibox, int sn);
    virtual int getSnFromBox(netkit::IBox* ibox);

protected:
    // 继承后可以修改
    virtual void onEvent(Event *event);
    virtual void onCheckRspTimeout();

    void loopEvents();
    void pushEvent(Event *event);

    int newBoxSn();

    void cocosScheduleLoopEvents();
    void cocosScheduleRspTimeoutCheck();

    void handleWithRspCallbacks(Event *event);
    void handleWithEventCallbacks(Event *event);

    void cocosUnScheduleAll();

protected:
    ferry::Service m_service;

    pthread_mutex_t m_eventsMutex;
    std::list<Event*> m_events;

    bool m_running;

private:
    int m_boxSn;

    std::map<void*, std::map<std::string, CallbackType> > m_mapEventCallbacks;
    // 最先过期的排在最左边
    std::list<RspCallbackContainer> m_listRspCallbacks;
};

}

#endif //__Ferry_2014_10_22_H_
