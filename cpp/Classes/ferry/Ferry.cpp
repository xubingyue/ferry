//
// Created by dantezhu on 14-10-22.
//

#include "Ferry.h"
#include "netkit/Box.h"

namespace ferry {
Ferry *Ferry::getInstance() {
    // 用指针可以保证进程结束时，释放晚一些，不会报错
    static Ferry *instance;
    if (!instance) {
        instance = new Ferry();
    }
    return instance;
}

Ferry::Ferry() {
    pthread_mutex_init(&m_eventsMutex, NULL);
    pthread_mutex_init(&m_boxSnMutex, NULL);

    m_boxSn = 0;
}

Ferry::~Ferry() {
    stop();
    delAllCallbacks();
    clearEvents();

    pthread_mutex_destroy(&m_eventsMutex);
    pthread_mutex_destroy(&m_boxSnMutex);
}

ferry::Service*Ferry::getService() {
    return &m_service;
}

int Ferry::init(const std::string& host, short port) {
    return m_service.init(this, host, port);
}

void Ferry::start() {
    if (isRunning()) {
        return;
    }

    cocosSchedule();

    m_service.start();
}

void Ferry::stop() {
    if (!isRunning()) {
        return;
    }

    m_service.stop();

    cocosUnSchedule();
}

void Ferry::connect() {
    m_service.connect();
}

void Ferry::disconnect() {
    m_service.disconnect();
}

void Ferry::pauseSchedule() {
    cocos2d::Director::getInstance()->getScheduler()->pauseTarget(this);
}

void Ferry::resumeSchedule() {
    cocos2d::Director::getInstance()->getScheduler()->resumeTarget(this);
}

bool Ferry::isSchedulePaused() {
    return cocos2d::Director::getInstance()->getScheduler()->isTargetPaused(this);
}

bool Ferry::isConnected() {
    return m_service.isConnected();
}

bool Ferry::isRunning() {
    return m_service.isRunning();
}

void Ferry::delCallbacksForTarget(void *target) {
    delEventCallbacksForTarget(target);
    delRspCallbacksForTarget(target);
}

void Ferry::delAllCallbacks() {
    delAllEventCallbacks();
    delAllRspCallbacks();
}

void Ferry::send(netkit::IBox *box) {
    int sn = newBoxSn();
    setSnToBox(box, sn);

    m_service.send(box);
}

void Ferry::send(netkit::IBox *box, CallbackType callback, float timeout, void* target) {
    int sn = newBoxSn();
    setSnToBox(box, sn);

    struct timeval tvNow, tvTimeout;

    // 当前时间
    gettimeofday(&tvNow, NULL);

    // 超时
    tvTimeout.tv_sec = (int)(timeout);
    tvTimeout.tv_usec = (int)((timeout - tvTimeout.tv_sec) * 1000000);

    RspCallbackContainer *container = new RspCallbackContainer();
    container->sn = sn;

    timeradd(&tvNow, &tvTimeout, &container->expireTime);
    container->callback = callback;
    container->target = target;

    for (auto it = m_rspCallbacks.begin();; it ++) {
        if (it == m_rspCallbacks.end() ||
                timercmp(&(*it)->expireTime, &container->expireTime, >) ) {
            m_rspCallbacks.insert(it, container);
            break;
        }
    }

    m_service.send(box);
}

void Ferry::delRspCallbacksForTarget(void *target) {
    for(auto it = m_rspCallbacks.begin(); it != m_rspCallbacks.end();) {
        auto& container = *it;
        if (container->target == target) {
            // 先delete
            delete container;
            // 移除
            it = m_rspCallbacks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Ferry::delAllRspCallbacks() {
    for (auto& container: m_rspCallbacks) {
        delete container;
    }

    m_rspCallbacks.clear();
}

void Ferry::addEventCallback(CallbackType callback, void *target, const std::string &name) {
    EventCallbackContainer *container = new EventCallbackContainer();
    container->callback = callback;
    container->target = target;
    container->name = name;

    m_eventCallbacks.push_back(container);
}

void Ferry::delEventCallback(const std::string &name, void *target) {
    for(auto it = m_eventCallbacks.begin(); it != m_eventCallbacks.end();) {
        auto& container = *it;
        if (container->name == name && container->target == target) {
            delete container;
            // 移除
            it = m_eventCallbacks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Ferry::delEventCallbacksForTarget(void *target) {
    for(auto it = m_eventCallbacks.begin(); it != m_eventCallbacks.end();) {
        auto& container = *it;
        if (container->target == target) {
            delete container;
            // 移除
            it = m_eventCallbacks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Ferry::delAllEventCallbacks() {
    for(auto& container: m_eventCallbacks) {
        delete container;
    }

    m_eventCallbacks.clear();
}

void Ferry::onOpen(ferry::Service *service) {
    cocos2d::log("[%s][%d][%s]", __FILE__, __LINE__, __FUNCTION__);

    Event *event = new Event();
    event->what = EVENT_OPEN;
    postEvent(event);
}

void Ferry::onSend(ferry::Service *service, netkit::IBox *ibox) {
    cocos2d::log("[%s][%d][%s] box: %s", __FILE__, __LINE__, __FUNCTION__, ibox->toString().c_str());
    Event *event = new Event();
    event->what = EVENT_SEND;
    event->box = (netkit::Box*)ibox;
    postEvent(event);
}

void Ferry::onRecv(ferry::Service *service, netkit::IBox *ibox) {
    cocos2d::log("[%s][%d][%s] box: %s", __FILE__, __LINE__, __FUNCTION__, ibox->toString().c_str());

    Event *event = new Event();
    event->what = EVENT_RECV;
    event->box = (netkit::Box*)ibox;
    postEvent(event);
}

void Ferry::onClose(ferry::Service *service) {
    cocos2d::log("[%s][%d][%s]", __FILE__, __LINE__, __FUNCTION__);

    Event *event = new Event();
    event->what = EVENT_CLOSE;
    postEvent(event);
}

void Ferry::onError(ferry::Service *service, int code, netkit::IBox *ibox) {
    cocos2d::log("[%s][%d][%s] code: %d, box: %s",
        __FILE__, __LINE__, __FUNCTION__, code, ibox ? ibox->toString().c_str() : "null");

    Event *event = new Event();
    event->what = EVENT_ERROR;
    event->code = code;
    event->box = ibox;
    postEvent(event);
}

netkit::IBox* Ferry::createBox() {
    return new netkit::Box();
}

void Ferry::releaseBox(netkit::IBox* ibox) {
    if (ibox) {
        delete ibox;
    }
}

void Ferry::setSnToBox(netkit::IBox* ibox, int sn) {
    netkit::Box* box = (netkit::Box*)ibox;
    box->sn = sn;
}

int Ferry::getSnFromBox(netkit::IBox* ibox) {
    netkit::Box* box = (netkit::Box*)ibox;
    return box->sn;
}

void Ferry::loopEvents() {
    pthread_mutex_lock(&m_eventsMutex);
    // 防止访问冲突
    std::list<Event*> events;
    // 相当于直接剪切，速度快很多
    events.splice(events.end(), m_events);
    pthread_mutex_unlock(&m_eventsMutex);

    for (auto& event: events) {
        onEvent(event);
        delete event;
    }
}

void Ferry::postEvent(Event *event) {
    pthread_mutex_lock(&m_eventsMutex);

    m_events.push_back(event);

    pthread_mutex_unlock(&m_eventsMutex);
}

void Ferry::clearEvents() {
    pthread_mutex_lock(&m_eventsMutex);

    for (auto& event: m_events) {
        delete event;
    }

    m_events.clear();

    pthread_mutex_unlock(&m_eventsMutex);
}

void Ferry::onEvent(Event *event) {
    if (event->box && getSnFromBox(event->box) > 0) {
        // SEND, RECV, ERROR
        handleWithRspCallbacks(event);
    }
    else {
        // eventCallback只接受sn=0的，至于跨界面通信，这个由应用自己走eventbus
        handleWithEventCallbacks(event);
    }
}

int Ferry::newBoxSn() {
    pthread_mutex_lock(&m_boxSnMutex);

    m_boxSn %= 2100000000;

    // 即最小也是1
    int boxSn = ++m_boxSn;

    pthread_mutex_unlock(&m_boxSnMutex);

    return boxSn;
}

void Ferry::cocosSchedule() {
    auto func = [this](float dt){
        loopEvents();

        checkRspTimeout();
    };

    // 先调用这个
    cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, __FUNCTION__);
}

void Ferry::checkRspTimeout() {
    if (m_rspCallbacks.empty()) {
        return;
    }

    struct timeval tvNow;
    gettimeofday(&tvNow, NULL);

    std::list<RspCallbackContainer*> todoCallbacks;

    // 提前申请好，免得每次都要传
    // 因为纯删除，且挡住主线程，不会导致结构被破坏
    for(auto& container: m_rspCallbacks) {
        if (timercmp(&tvNow, &container->expireTime, >)) {
            todoCallbacks.push_back(container);
        }
        else {
            // 找到第一个没超时的，那么后面就都没有超时了
            break;
        }
    }

    if (todoCallbacks.empty()) {
        // 无需处理
        return;
    }

    // 再进入下一帧之前，不会释放
    Event *event = new Event();
    event->what = EVENT_TIMEOUT;

    for (auto& container: todoCallbacks) {
        // 没找到
        if (std::find(m_rspCallbacks.begin(), m_rspCallbacks.end(), container) == m_rspCallbacks.end()) {
            continue;
        }

        container->callback(event);

        delete container;

        m_rspCallbacks.remove(container);
    }

    delete event;
}

void Ferry::handleWithRspCallbacks(Event *event) {
    auto rspCallbacks = m_rspCallbacks;

    int sn = getSnFromBox(event->box);

    for(auto it = rspCallbacks.begin(); it != rspCallbacks.end(); ++it) {
        auto& container = *it;

        if (std::find(m_rspCallbacks.begin(), m_rspCallbacks.end(), container) == m_rspCallbacks.end()) {
            continue;
        }

        if (container->sn == sn)
        {
            // 调用
            container->callback(event);
            // 移除
            if (event->what == EVENT_RECV || event->what == EVENT_ERROR) {
                delete container;
                m_rspCallbacks.remove(container);
            }
        }
    }
}

void Ferry::handleWithEventCallbacks(Event *event) {
    auto eventCallbacks = m_eventCallbacks;

    for(auto it = eventCallbacks.begin(); it != eventCallbacks.end(); ++it) {
        auto& container = *it;
        if (std::find(m_eventCallbacks.begin(), m_eventCallbacks.end(), container) == m_eventCallbacks.end()) {
            // 已经中途被释放了
            continue;
        }

        container->callback(event);
    }
}

void Ferry::cocosUnSchedule() {
    cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
}
}
