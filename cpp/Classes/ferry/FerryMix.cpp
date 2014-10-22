//
// Created by dantezhu on 14-10-22.
//

#include "FerryMix.h"
#include "Box.h"
#include "cocos2d.h"

namespace ferry {
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
    netkit::Box *box;
    int errcode;
};

FerryMix* FerryMix::getInstance() {
    static FerryMix instance;
    return &instance;
}

FerryMix::FerryMix() {
    m_boxSn = 0;
    m_timeoutCheckInterval = TIMEOUT_CHECK_INTERVAL;

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
    scheduleEventBusLoop();
    scheduleRspTimeoutCheckLoop();

    m_service.start();
}

void FerryMix::stop() {
    m_service.stop();
}

void FerryMix::connect() {
    m_service.connect();
}

void FerryMix::send(netkit::Box *box) {
    m_service.send(box);
}

void FerryMix::send(netkit::Box *box, rsp_callback_type rsp_callback, float timeout) {
    int sn = newBoxSn();

    box->sn = sn;

    RspCallbackContainer callbackContainer;
    callbackContainer.timeout = timeout;
    callbackContainer.callback = rsp_callback;
    gettimeofday(&callbackContainer.createTime, NULL);

    m_mapRspCallbacks[sn] = callbackContainer;

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

    if(e->what == EVENT_ON_RECV) {
        InnerEvent* event = (InnerEvent*)e;
        handleRsp(event->box);
    }

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
    e->box = (netkit::Box*)ibox;
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

void FerryMix::scheduleEventBusLoop() {
    auto func = [this](float dt){
        m_eventBus.loopEvents();
    };

    // 先调用这个
    cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, "ferry_mix_eventbus");
}

void FerryMix::scheduleRspTimeoutCheckLoop() {
    auto func = [this](float dt){
        checkRspTimeout();
    };

    // 先调用这个
    cocos2d::Director::getInstance()->getScheduler()->schedule(
            func, this, m_timeoutCheckInterval, false, "ferry_mix_timeout_check"
    );
}

void FerryMix::checkRspTimeout() {

    struct timeval nowTime;

    gettimeofday(&nowTime, NULL);

    for(auto it = m_mapRspCallbacks.begin(); it != m_mapRspCallbacks.end();) {
        auto& container = it->second;
        float past = decTime(nowTime, container.createTime);
        auto tempit = it;
        it++;
        if (past > container.timeout)
        {
            // 超时了
            container.callback(ERROR_TIMEOUT, nullptr);

            // 移除
            m_mapRspCallbacks.erase(tempit);
        }

    }
}

void FerryMix::handleRsp(netkit::Box* box) {
    if (m_mapRspCallbacks.find(box->sn) == m_mapRspCallbacks.end()) {
        // 没有找到
        return;
    }

    auto& container = m_mapRspCallbacks[box->sn];
    container.callback(box->ret, box);

    m_mapRspCallbacks.erase(box->sn);
}

float FerryMix::decTime(struct timeval &first, struct timeval &second) {
    long long intervalMS = ((long long)(first.tv_sec  - second.tv_sec ) * 1000000 + (first.tv_usec - second.tv_usec)) / 1000;

    // 强转float
    return (float)(intervalMS / 1000.0);
}

}
