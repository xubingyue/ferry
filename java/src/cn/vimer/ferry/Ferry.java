package cn.vimer.ferry;

import android.os.Handler;
import android.os.Message;
import cn.vimer.netkit.Box;
import cn.vimer.netkit.IBox;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by dantezhu on 14-12-16.
 */
public class Ferry implements Delegate {
    private static boolean debug = false;
    private static Ferry ferryInstance;
    private Service service;
    private int boxSn;
    private Lock boxSnLock;
    private List<CallbackListener> rspCallbacks;
    private List<CallbackListener> eventCallbacks;
    private Handler timeoutChecker;
    private Handler eventHandler;

    public Ferry() {
        service = new Service();
        boxSnLock = new ReentrantLock();
        rspCallbacks = new LinkedList<CallbackListener>();
        eventCallbacks = new LinkedList<CallbackListener>();
        timeoutChecker = new Handler();

        eventHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                onMessageMainThread(msg);
                super.handleMessage(msg);
            }
        };

    }

    public static Ferry getInstance() {
        if (ferryInstance == null) {
            ferryInstance = new Ferry();
        }

        return ferryInstance;
    }

    public static void setDebug(boolean debug_) {
        debug = debug_;

        if (debug) {
            FLog.setLevel(FLog.DEBUG);
        }
        else {
            FLog.setLevel(FLog.ERROR);
        }
    }

    public static boolean getDebug() {
        return debug;
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

    public long getLastActiveTimeMills() {
        return service.getLastActiveTimeMills();
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

        for (int i=0; ; i++) {
            if (i >= rspCallbacks.size() ||
                    listener.expireTime < rspCallbacks.get(i).expireTime) {
                // 插入到i前面
                rspCallbacks.add(i, listener);
                break;
            }
        }

        service.send(ibox);
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
        FLog.d("");
        eventHandler.sendEmptyMessage(Constants.EVENT_OPEN);
    }

    public void onSend(Service service, IBox ibox) {
        FLog.d(String.format("box: %s", ibox));
        Message msg = new Message();
        msg.what = Constants.EVENT_SEND;
        msg.obj = ibox;

        eventHandler.sendMessage(msg);
    }

    public void onRecv(Service service, IBox ibox) {
        FLog.d(String.format("box: %s", ibox));
        Message msg = new Message();
        msg.what = Constants.EVENT_RECV;
        msg.obj = ibox;

        eventHandler.sendMessage(msg);
    }

    public void onClose(Service service) {
        FLog.d("");
        eventHandler.sendEmptyMessage(Constants.EVENT_CLOSE);
    }

    public void onError(Service service, int code, IBox ibox) {
        FLog.d(String.format("code: %s, box: %s", code, ibox));
        Message msg = new Message();
        msg.what = Constants.EVENT_ERROR;
        msg.arg1 = code;
        msg.obj = ibox;

        eventHandler.sendMessage(msg);
    }

    public void onTimeout(Service service) {
        FLog.d(String.format("code: %s, box: %s", code, ibox));
        Message msg = new Message();
        msg.what = Constants.EVENT_TIMEOUT;

        eventHandler.sendMessage(msg);
    }

    public IBox createBox() {
        return new Box();
    }

    public void onMessageMainThread(Message msg) {
        if (msg.obj != null && getSnFromBox((IBox)msg.obj) > 0) {
            handleWithRspCallbacks(msg);
        }
        else {
            handleWithEventCallbacks(msg);
        }
    }

    private void handleWithRspCallbacks(Message msg) {
        int sn = getSnFromBox((IBox)msg.obj);
        LinkedList<CallbackListener> tmpListeners = new LinkedList<CallbackListener>(rspCallbacks);

        for (CallbackListener listener: tmpListeners) {
            if (rspCallbacks.indexOf(listener) < 0) {
                // 没找到，说明被删除了
                continue;
            }

            if (listener.sn == sn) {
                callListener(listener, msg);

                if (msg.what == Constants.EVENT_RECV || msg.what == Constants.EVENT_ERROR) {
                    rspCallbacks.remove(listener);
                }
            }
        }
    }

    private void handleWithEventCallbacks(Message event) {
        LinkedList<CallbackListener> tmpListeners = new LinkedList<CallbackListener>(eventCallbacks);

        for (CallbackListener listener: tmpListeners) {
            if (eventCallbacks.indexOf(listener) < 0) {
                // 没找到，说明被删除了
                continue;
            }

            callListener(listener, event);
        }
    }

    private void callListener(CallbackListener listener, Message msg) {
        IBox box = (IBox) msg.obj;
        switch (msg.what) {
            case Constants.EVENT_OPEN:
                listener.onOpen();
                break;
            case Constants.EVENT_SEND:
                listener.onSend(box);
                break;
            case Constants.EVENT_RECV:
                listener.onRecv(box);
                break;
            case Constants.EVENT_CLOSE:
                listener.onClose();
                break;
            case Constants.EVENT_ERROR:
                listener.onError(msg.arg1, box);
                break;
            case Constants.EVENT_TIMEOUT:
                listener.onTimeout();
                break;
        }
    }

    private int newBoxSn() {
        boxSnLock.lock();
        try {
            boxSn %= 2100000000;
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

        for (CallbackListener listener: todoListeners) {
            FLog.d(String.format("timeout. sn: %s", listener.sn));

            listener.onTimeout();
            rspCallbacks.remove(listener);
        }
    }

    // 加static，才可以在外面访问
    public static class CallbackListener {
        private String name;
        private Object target;
        private int sn;
        private long expireTime;

        public void onOpen() {}

        public void onSend(IBox ibox) {}

        public void onRecv(IBox ibox) {}

        public void onClose() {}

        public void onError(int code, IBox ibox) {}

        public void onTimeout() {}
    }
}
