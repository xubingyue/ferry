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

// 如果cocos2d.h不放在最前，win下会编译报错
#include "cocos2d.h"
#include "EventBus.h"
#include "Delegate.h"
#include "Service.h"
#include "IBox.h"

namespace ferry {

enum EVENT_TYPE {
    EVENT_ON_OPEN=0,
    EVENT_ON_RECV,
    EVENT_ON_CLOSE,
    EVENT_ON_ERROR,
};

class Event :public eventbus::BaseEvent{
public:
    Event() {
        box = nullptr;
        code = 0;
    }
    virtual ~Event() {
        if(box) {
            delete box;
        }
        box = nullptr;
    }

public:
    netkit::IBox *box;
    int code;
};

// 超时的错误码
const int RSP_ERROR_TIMEOUT = -999;

// 超时检查间隔
const float TIMEOUT_CHECK_INTERVAL = 1.0;

// 事件注册的回调
typedef std::function<void(Event*)> event_callback_type;

// 收到服务器响应的回调
typedef std::function<void(int, netkit::IBox*)> rsp_callback_type;

struct RspCallbackContainer {
    time_t createTime;
    float timeout;
    rsp_callback_type callback;
    void* target;
};

class Ferry : public eventbus::IHandler, public ferry::Delegate {
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
    void delAllCallbacksForTarget(void* target);
    // 删除所有回调
    void delAllCallbacks();

    // 发送消息
    int send(netkit::IBox *box);
    // 带回调的发送，以及超时，超时为秒。target很有用，可以用来防止崩溃
    int send(netkit::IBox *box, rsp_callback_type rsp_callback, float timeout, void* target);

    // 删除send对应的回调
    void delAllRspCallbacksForTarget(void* target);
    void delAllRspCallbacks();

    // 注册事件回调
    void addEventCallback(event_callback_type callback, void* target, const std::string& name);
    void delEventCallback(const std::string& name, void* target);
    void delAllEventCallbacksForTarget(void* target);
    void delAllEventCallback();


    // IHandler begin
    virtual void onEvent(eventbus::BaseEvent *e);
    // IHandler end

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
    int newBoxSn();

    void cocosScheduleEventBusLoop();
    void cocosScheduleRspTimeoutCheckLoop();

    void checkRspTimeout();

    void handleRsp(netkit::IBox* box);

    void cocosUnScheduleAll();

private:
    eventbus::EventBus m_eventBus;
    ferry::Service m_service;

    std::map<void*, std::map<std::string, event_callback_type> > m_mapEventCallbacks;

    std::map<int, RspCallbackContainer> m_mapRspCallbacks;

    int m_boxSn;

    float m_timeoutCheckInterval;

    bool m_running;
};

}

#endif //__Ferry_2014_10_22_H_
