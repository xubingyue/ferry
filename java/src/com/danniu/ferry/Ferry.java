package com.danniu.ferry;

import android.os.Handler;
import com.danniu.netkit.Box;
import com.danniu.netkit.IBox;
import de.greenrobot.event.EventBus;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by dantezhu on 14-12-16.
 */
public class Ferry implements Delegate {
    private Service service;
    private EventBus eventBus;
    private int boxSn;
    private Lock boxSnLock;
    private List<CallbackListener> rspCallbacks;
    private List<CallbackListener> eventCallbacks;
    private Handler timeoutChecker;

    public Ferry() {
        service = new Service();
        eventBus = new EventBus();
        eventBus.register(this);
        boxSnLock = new ReentrantLock();
        rspCallbacks = new LinkedList<CallbackListener>();
        eventCallbacks = new LinkedList<CallbackListener>();
        timeoutChecker = new Handler();
    }

    public int init(String host, int port) {
        // 准备好
        initTimeoutChecker();
        return service.init(this, host, port);
    }

    public void start() {
        if (isRunning()) {
            return;
        }

        service.start();
    }

    public void stop() {
        if (!isRunning()) {
            return;
        }

        service.stop();
    }

    public void connect() {
        service.connect();
    }

    public void disconnect() {
        service.disconnect();
    }

    public boolean isConnected() {
        return service.isConnected();
    }

    public boolean isRunning() {
        return service.isRunning();
    }

    public void delCallbacksForTarget(Object target) {
        delRspCallbacksForTarget(target);
        delEventCallbacksForTarget(target);
    }

    public void delAllCallbacks() {
        delAllRspCallbacks();
        delAllEventCallbacks();
    }

    public void send(IBox ibox) {
        int sn = newBoxSn();
        setSnToBox(ibox, sn);

        service.send(ibox);
    }

    public void send(IBox ibox, CallbackListener listener, float timeout, Object target) {
        int sn = newBoxSn();
        setSnToBox(ibox, sn);

        listener.target = target;
        listener.expireTime = (long)(timeout * 1000) + System.currentTimeMillis();
        listener.sn = sn;

        for (int i=0; i<rspCallbacks.size(); i++) {
            if (i == rspCallbacks.size()-1 ||
                    listener.expireTime < rspCallbacks.get(i).expireTime) {
                // 插入到i前面
                rspCallbacks.add(i, listener);
                break;
            }
        }
    }

    public void delRspCallbacksForTarget(Object target) {
        Iterator<CallbackListener> iterator = rspCallbacks.iterator();
        while (iterator.hasNext()) {
            CallbackListener listener = iterator.next();
            if (listener.target == target) {
                // 这种删除是安全的
                iterator.remove();
            }
        }
    }

    public void delAllRspCallbacks() {
        rspCallbacks.clear();
    }

    public void addEventCallback(CallbackListener listener, Object target, String name) {
        listener.target = target;
        listener.name = name;

        eventCallbacks.add(listener);
    }

    public void delEventCallback(String name, Object target) {
        Iterator<CallbackListener> iterator = eventCallbacks.iterator();
        while (iterator.hasNext()) {
            CallbackListener listener = iterator.next();
            if (listener.name.equals(name) && listener.target == target) {
                // 这种删除是安全的
                iterator.remove();
            }
        }
    }

    public void delEventCallbacksForTarget(Object target) {
        Iterator<CallbackListener> iterator = eventCallbacks.iterator();
        while (iterator.hasNext()) {
            CallbackListener listener = iterator.next();
            if (listener.target == target) {
                // 这种删除是安全的
                iterator.remove();
            }
        }
    }

    public void delAllEventCallbacks() {
        eventCallbacks.clear();
    }

    public void onOpen(Service service) {
        eventBus.post(new Event(Constants.EVENT_OPEN));
    }

    public void onSend(Service service, IBox ibox) {
        Event event = new Event(Constants.EVENT_SEND);
        event.box = ibox;

        eventBus.post(event);
    }

    public void onRecv(Service service, IBox ibox) {
        Event event = new Event(Constants.EVENT_RECV);
        event.box = ibox;

        eventBus.post(event);
    }

    public void onClose(Service service) {
        eventBus.post(new Event(Constants.EVENT_CLOSE));
    }

    public void onError(Service service, int code, IBox ibox) {
        Event event = new Event(Constants.EVENT_ERROR);
        event.code = code;
        event.box = ibox;

        eventBus.post(event);
    }

    public IBox createBox() {
        return new Box();
    }

    public void onEventMainThread(Event event) {
        if (event.box != null && getSnFromBox(event.box) > 0) {
            handleWithRspCallbacks(event);
        }
        else {
            handleWithEventCallbacks(event);
        }
    }

    private void handleWithRspCallbacks(Event event) {
        int sn = getSnFromBox(event.box);
        LinkedList<CallbackListener> tmpListeners = new LinkedList<CallbackListener>(rspCallbacks);

        for (CallbackListener listener: tmpListeners) {
            if (rspCallbacks.indexOf(listener) < 0) {
                // 没找到，说明被删除了
                continue;
            }

            if (listener.sn == sn) {
                callListener(listener, event);

                rspCallbacks.remove(listener);
            }
        }
    }

    private void handleWithEventCallbacks(Event event) {
        LinkedList<CallbackListener> tmpListeners = new LinkedList<CallbackListener>(eventCallbacks);

        for (CallbackListener listener: tmpListeners) {
            if (eventCallbacks.indexOf(listener) < 0) {
                // 没找到，说明被删除了
                continue;
            }

            callListener(listener, event);
        }
    }

    private void callListener(CallbackListener listener, Event event) {
        switch (event.what) {
            case Constants.EVENT_OPEN:
                listener.onOpen();
                break;
            case Constants.EVENT_CLOSE:
                listener.onClose();
                break;
            case Constants.EVENT_SEND:
                listener.onSend(event.box);
                break;
            case Constants.EVENT_RECV:
                listener.onRecv(event.box);
                break;
            case Constants.EVENT_ERROR:
                listener.onError(event.code, event.box);
                break;
            case Constants.EVENT_TIMEOUT:
                listener.onTimeout();
                break;
        }
    }

    private int newBoxSn() {
        boxSn %= 2100000000;

        boxSnLock.lock();
        try {
            return ++boxSn;
        }
        finally {
            boxSnLock.unlock();
        }
    }

    private void setSnToBox(IBox ibox, int sn) {
        Box box = (Box)ibox;
        box.sn = sn;
    }

    private int getSnFromBox(IBox ibox) {
        return ((Box)ibox).sn;
    }

    private void initTimeoutChecker() {
        timeoutChecker.postDelayed(new Runnable() {
            @Override
            public void run() {
                checkRspTimeout();
                timeoutChecker.postDelayed(this, Constants.TIMEOUT_CHECK_INTERVAL_MS);
            }
        }, Constants.TIMEOUT_CHECK_INTERVAL_MS);
    }

    private void checkRspTimeout() {
        if (rspCallbacks.isEmpty()) {
            return;
        }

        long nowTime = System.currentTimeMillis();

        LinkedList<CallbackListener> todoListeners = new LinkedList<CallbackListener>();

        for(CallbackListener listener: rspCallbacks) {
            if (listener.expireTime < nowTime) {
                // 说明超时了
                todoListeners.add(listener);
            }
            else {
                break;
            }
        }

        if (todoListeners.isEmpty()) {
            return;
        }

        Event event = new Event(Constants.EVENT_TIMEOUT);

        for (CallbackListener listener: todoListeners) {
            callListener(listener, event);
            rspCallbacks.remove(listener);
        }
    }

    public class CallbackListener {
        public String name;
        public Object target;
        public int sn;
        public long expireTime;

        public void onOpen() {}

        public void onSend(IBox ibox) {}

        public void onRecv(IBox ibox) {}

        public void onClose() {}

        public void onError(int code, IBox ibox) {}

        public void onTimeout() {}
    }

    public class Event {
        public int what;
        public IBox box;
        public int code;

        Event(int what_) {
            what = what_;
        }
    }
}
