//
// Created by dantezhu on 14-10-22.
//

#include "Ferry.h"
#include "Box.h"

#define LOG_TAG "ferry"

namespace ferry {
Ferry *Ferry::getInstance() {
    static Ferry instance;
    return &instance;
}

Ferry::Ferry() {
    m_boxSn = 0;
    m_running = false;
    m_timeoutCheckInterval = TIMEOUT_CHECK_INTERVAL;

    m_eventBus.addHandler(this);
}

Ferry::~Ferry() {
    stop();

    m_eventBus.delHandler(this);
}

ferry::Service*Ferry::getService() {
    return &m_service;
}

int Ferry::init(const std::string& host, short port) {
    return m_service.init(this, host, port);
}

void Ferry::start() {
    if (m_running) {
        return;
    }
    m_running = true;

    cocosScheduleEventBusLoop();
    cocosScheduleRspTimeoutCheckLoop();

    m_service.start();
}

void Ferry::stop() {
    if (!m_running) {
        return;
    }
    m_running = false;

    m_service.stop();

    cocosUnScheduleAll();
}

void Ferry::connect() {
    m_service.connect();
}

void Ferry::delCallbacksForTarget(void *target) {
    delEventCallbacksForTarget(target);
    delRspCallbacksForTarget(target);
}

void Ferry::delAllCallbacks() {
    delAllEventCallbacks();
    delAllRspCallbacks();
}

int Ferry::send(netkit::IBox *box) {
    return m_service.send(box);
}

int Ferry::send(netkit::IBox *box, CallbackType rsp_callback, float timeout, void* target) {
    int sn = newBoxSn();

    setSnToBox(box, sn);

    RspCallbackContainer callbackContainer;
    callbackContainer.timeout = timeout;
    callbackContainer.callback = rsp_callback;
    callbackContainer.target = target;
    callbackContainer.createTime = time(NULL);

    m_mapRspCallbacks[sn] = callbackContainer;

    return m_service.send(box);
}

void Ferry::delRspCallbacksForTarget(void *target) {
    for(auto it = m_mapRspCallbacks.begin(); it != m_mapRspCallbacks.end();) {
        auto& container = it->second;
        auto tempit = it;
        it++;
        if (container.target == target)
        {
            // 移除
            m_mapRspCallbacks.erase(tempit);
        }

    }
}

void Ferry::delAllRspCallbacks() {
    m_mapRspCallbacks.clear();
}

void Ferry::addEventCallback(CallbackType callback, void *target, const std::string &name) {
    m_mapEventCallbacks[target][name]= callback;
}

void Ferry::delEventCallback(const std::string &name, void *target) {
    if(m_mapEventCallbacks.find(target) == m_mapEventCallbacks.end()){
        return;
    }

    m_mapEventCallbacks[target].erase(name);
}

void Ferry::delEventCallbacksForTarget(void *target) {
    m_mapEventCallbacks.erase(target);
}

void Ferry::delAllEventCallbacks() {
    m_mapEventCallbacks.clear();
}


void Ferry::onEvent(eventbus::BaseEvent *event) {
    Event* evt = (Event *) event;

    if(evt->what == EVENT_RECV) {
        handleRsp(evt);
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

            subit->second(evt);
        }
    }
}

void Ferry::onOpen(ferry::Service *service) {
    cocos2d::log("[%s]-[%s][%d][%s]", LOG_TAG, __FILE__, __LINE__, __FUNCTION__);

    Event *event = new Event();
    event->what = EVENT_OPEN;
    m_eventBus.pushEvent(event);
}

void Ferry::onSend(ferry::Service *service, netkit::IBox *ibox) {
    cocos2d::log("[%s]-[%s][%d][%s] box: %s", LOG_TAG, __FILE__, __LINE__, __FUNCTION__, ibox->toString().c_str());
}

void Ferry::onRecv(ferry::Service *service, netkit::IBox *ibox) {
    cocos2d::log("[%s]-[%s][%d][%s] box: %s", LOG_TAG, __FILE__, __LINE__, __FUNCTION__, ibox->toString().c_str());

    Event *event = new Event();
    event->what = EVENT_RECV;
    event->box = (netkit::Box*)ibox;
    m_eventBus.pushEvent(event);
}

void Ferry::onClose(ferry::Service *service) {
    cocos2d::log("[%s]-[%s][%d][%s]", LOG_TAG, __FILE__, __LINE__, __FUNCTION__);

    Event *event = new Event();
    event->what = EVENT_CLOSE;
    m_eventBus.pushEvent(event);
}

void Ferry::onError(ferry::Service *service, int code) {
    cocos2d::log("[%s]-[%s][%d][%s] code: %d", LOG_TAG, __FILE__, __LINE__, __FUNCTION__, code);

    Event *event = new Event();
    event->what = EVENT_ERROR;
    event->code = code;
    m_eventBus.pushEvent(event);
}

netkit::IBox*Ferry::createBox() {
    return new netkit::Box();
}

void Ferry::setSnToBox(netkit::IBox* ibox, int sn) {
    netkit::Box* box = (netkit::Box*)ibox;
    box->sn = sn;
}

int Ferry::getSnFromBox(netkit::IBox* ibox) {
    netkit::Box* box = (netkit::Box*)ibox;
    return box->sn;
}


int Ferry::newBoxSn() {
    if (m_boxSn <= 0) {
        m_boxSn = 0;
    }

    // 即最小也是1
    return ++m_boxSn;
}

void Ferry::cocosScheduleEventBusLoop() {
    auto func = [this](float dt){
        m_eventBus.loopEvents();
    };

    // 先调用这个
    cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, "ferry_eventbus");
}

void Ferry::cocosScheduleRspTimeoutCheckLoop() {
    auto func = [this](float dt){
        checkRspTimeout();
    };

    // 先调用这个
    cocos2d::Director::getInstance()->getScheduler()->schedule(
            func, this, m_timeoutCheckInterval, false, "ferry_timeout_check"
    );
}

void Ferry::checkRspTimeout() {

    time_t nowTime = time(NULL);

    // 提前申请好，免得每次都要传
    Event *event = new Event();
    event->what = EVENT_TIMEOUT;

    for(auto it = m_mapRspCallbacks.begin(); it != m_mapRspCallbacks.end();) {
        auto& container = it->second;
        float past = nowTime - container.createTime;
        auto tempit = it;
        it++;
        if (past > container.timeout)
        {
            // 超时了
            container.callback(event);
            // 用完就删掉

            // 移除
            m_mapRspCallbacks.erase(tempit);
        }
    }

    delete event;
}

void Ferry::handleRsp(Event* event) {
    int sn = getSnFromBox(event->box);

    if (m_mapRspCallbacks.find(sn) == m_mapRspCallbacks.end()) {
        // 没有找到
        return;
    }

    auto& container = m_mapRspCallbacks[sn];
    container.callback(event);

    m_mapRspCallbacks.erase(sn);
}


void Ferry::cocosUnScheduleAll() {
    cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
}
}
