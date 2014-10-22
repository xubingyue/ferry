//
// Created by dantezhu on 14-10-22.
//

#include "FerryMix.h"
#include "Box.h"
#include "cocos2d.h"

namespace ferry {
enum EVENT_TYPE {
    EVENT_ON_OPEN=0,
    EVENT_ON_RECV,
    EVENT_ON_CLOSE,
    EVENT_ON_ERROR,
};

class InnerEvent :public eventbus::BaseEvent{
public:
    InnerEvent() {
        box = nullptr;
        errcode = 0;
    }
    virtual ~InnerEvent() {
        if(box) {
            delete box;
        }
        box = nullptr;
    }

public:
    netkit::IBox *box;
    int errcode;
};

FerryMix* FerryMix::getInstance() {
    static FerryMix instance;
    return &instance;
}

FerryMix::FerryMix() {
    m_boxSn = 0;
    m_eventBus.addHandler(this);
}

FerryMix::~FerryMix() {
    m_eventBus.delHandler(this);
}

ferry::Service* FerryMix::getService() {
    return &m_service;
}

int FerryMix::init(const std::string& host, short port) {
    return m_service.init(this, host, port);
}

void FerryMix::start() {
    auto func = [this](float dt){
        m_eventBus.loopEvents();
    };

    // 先调用这个
    cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, "ferry_mix_eventbus");

    m_service.start();
}

void FerryMix::stop() {
    m_service.stop();
}

void FerryMix::connect() {
    m_service.connect();
}

void FerryMix::send(netkit::IBox *box) {
    m_service.send(box);
}

void FerryMix::addEventCallback(event_callback_type callback, void *target, const std::string &name) {
    m_mapEventCallbacks[target][name]= callback;
}

void FerryMix::delEventCallback(const std::string &name, void *target) {
    if(m_mapEventCallbacks.find(target) == m_mapEventCallbacks.end()){
        return;
    }

    m_mapEventCallbacks[target].erase(name);
}

void FerryMix::delAllEventCallbacksForTarget(void *target) {
    m_mapEventCallbacks.erase(target);
}

void FerryMix::delAllEventCallback() {
    m_mapEventCallbacks.clear();
}


void FerryMix::onEvent(eventbus::BaseEvent *e) {
    auto mapEventCallbacks = m_mapEventCallbacks;

    for(auto it = mapEventCallbacks.begin(); it != mapEventCallbacks.end(); ++it) {
        auto target = it->first;
        if (m_mapEventCallbacks.find(target) == m_mapEventCallbacks.end()) {
            // 不用这个了
            continue;
        }

        // 这里注意是引用
        auto&real_callbacks = it->second;
        auto callbacks = real_callbacks;

        for(auto subit = callbacks.begin(); subit != callbacks.end(); ++subit) {
            auto name = subit->first;

            if (real_callbacks.find(name) == real_callbacks.end()) {
                continue;
            }

            try{
                subit->second(e);
            }
            catch(...)
            {
            }
        }
    }
}

void FerryMix::onOpen(ferry::Service *service) {
    InnerEvent * e = new InnerEvent();
    e->what = EVENT_ON_OPEN;
    m_eventBus.pushEvent(e);
}

void FerryMix::onSend(ferry::Service *service, netkit::IBox *ibox) {
    // 不能将box传出去，因为如果这样做，ibox那个时候可能已经释放了
}

void FerryMix::onRecv(ferry::Service *service, netkit::IBox *ibox) {
    InnerEvent * e = new InnerEvent();
    e->what = EVENT_ON_RECV;
    e->box = ibox;
    m_eventBus.pushEvent(e);
}

void FerryMix::onClose(ferry::Service *service) {
    InnerEvent * e = new InnerEvent();
    e->what = EVENT_ON_CLOSE;
    m_eventBus.pushEvent(e);
}

void FerryMix::onError(ferry::Service *service, int code) {
    InnerEvent * e = new InnerEvent();
    e->what = EVENT_ON_ERROR;
    e->errcode = code;
    m_eventBus.pushEvent(e);
}

netkit::IBox* FerryMix::createBox() {
    return new netkit::Box();
}


int FerryMix::newBoxSn() {
    if (m_boxSn <= 0) {
        m_boxSn = 0;
    }

    // 即最小也是1
    return ++m_boxSn;
}

}
