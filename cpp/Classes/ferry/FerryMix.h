//
// Created by dantezhu on 14-10-22.
//


#ifndef __FerryMix_2014_10_22_H_
#define __FerryMix_2014_10_22_H_

#include <iostream>
#include <map>
#include <set>
#include "EventBus.h"
#include "Delegate.h"

namespace ferry {

// 事件注册的回调
typedef std::function<void(eventbus::BaseEvent*)> event_callback_type;

// 收到服务器响应的回调
typedef std::function<void(netkit::IBox*)> rsp_callback_type;

class FerryMix : public eventbus::IHandler, public ferry::Delegate {
public:
    static FerryMix *getInstance();

    FerryMix();

    virtual ~FerryMix();

    ferry::Service* getService();

    int init(const std::string &host, short port);

    void start();

    void stop();

    void connect();

    // 发送消息
    void send(netkit::IBox *box);

    // 带回调的发送
    void send(netkit::IBox *box, rsp_callback_type rsp_callback);

    // 注册事件回调
    void addEventCallback(event_callback_type callback, void* target, const std::string& name);
    void delEventCallback(const std::string& name, void* target);
    void delAllEventCallback();
    void delAllEventCallbacksForTarget(void* target);


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

private:
    int newBoxSn();

private:
    eventbus::EventBus m_eventBus;
    ferry::Service m_service;

    std::map<void*, std::map<std::string, event_callback_type> > m_mapEventCallbacks;

    std::map<int, rsp_callback_type> m_mapRspCallbacks;

    int m_boxSn;
};

}

#endif //__FerryMix_2014_10_22_H_
