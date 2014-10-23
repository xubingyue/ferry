//
// Created by dantezhu on 14-10-22.
//


#ifndef __Ferry_2014_10_22_H_
#define __Ferry_2014_10_22_H_

#include <iostream>
#include <map>
#include <set>
#include <functional>
#include <time.h>
#include <pthread.h>

// 如果cocos2d.h不放在最前，win下会编译报错
#include "cocos2d.h"
#include "Delegate.h"
#include "Service.h"
#include "IBox.h"

namespace ferry {

enum EVENT_TYPE {
    EVENT_OPEN =0,
    EVENT_RECV,
    EVENT_CLOSE,
    EVENT_ERROR,
    EVENT_TIMEOUT,
};

class Event :public cocos2d::Ref {
private:
    Event() {
        what = 0;
        box = nullptr;
        code = 0;
        _done = false;
    }

public:
    virtual ~Event() {
        if(box) {
            delete box;
        }
        box = nullptr;
    }

    bool init() {
        return true;
    }

    CREATE_FUNC(Event);

public:
    int what;
    netkit::IBox *box;
    int code;

    bool _done;
};

// 超时检查间隔
const float TIMEOUT_CHECK_INTERVAL = 1.0;

// 事件注册的回调
typedef std::function<void(Event*)> CallbackType;

struct RspCallbackContainer {
    time_t createTime;
    float timeout;
    CallbackType callback;
    void* target;
};

class Ferry : public ferry::Delegate {
public:
    static Ferry *getInstance();

    Ferry();

    virtual ~Ferry();

    ferry::Service* getService();

    int init(const std::string &host, short port);

    void start();

    void stop();

    void connect();

    // 删除类对应的所有回调，务必在使用ferry的类的析构函数里调用
    void delCallbacksForTarget(void *target);
    // 删除所有回调
    void delAllCallbacks();

    // 发送消息
    int send(netkit::IBox *box);
    // 带回调的发送，以及超时，超时为秒。target很有用，可以用来防止崩溃
    int send(netkit::IBox *box, CallbackType rsp_callback, float timeout, void* target);

    // 删除send对应的回调
    void delRspCallbacksForTarget(void *target);
    void delAllRspCallbacks();

    // 注册事件回调
    void addEventCallback(CallbackType callback, void* target, const std::string& name);
    void delEventCallback(const std::string& name, void* target);
    void delEventCallbacksForTarget(void *target);
    void delAllEventCallbacks();

    // Delegate begin
    virtual void onOpen(ferry::Service *service);

    virtual void onSend(ferry::Service *service, netkit::IBox *ibox);

    virtual void onRecv(ferry::Service *service, netkit::IBox *ibox);

    virtual void onClose(ferry::Service *service);

    virtual void onError(ferry::Service *service, int code);

    virtual netkit::IBox *createBox();
    // Delegate end

    virtual void setSnToBox(netkit::IBox* ibox, int sn);
    virtual int getSnFromBox(netkit::IBox* ibox);

private:
    void loopEvents();
    void onEvent(Event *event);

    int newBoxSn();

    void cocosScheduleLoopEvents();
    void cocosScheduleRspTimeoutCheck();

    void checkRspTimeout();

    void handleRsp(Event* event);

    void cocosUnScheduleAll();

private:
    ferry::Service m_service;

    pthread_mutex_t m_eventsMutex;
    cocos2d::Vector<Event*> m_events;

    std::map<void*, std::map<std::string, CallbackType> > m_mapEventCallbacks;

    std::map<int, RspCallbackContainer> m_mapRspCallbacks;

    int m_boxSn;

    float m_timeoutCheckInterval;

    bool m_running;
};

}

#endif //__Ferry_2014_10_22_H_
