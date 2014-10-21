package com.danniu.demo;

import android.util.Log;
import com.danniu.ferry.Delegate;
import com.danniu.ferry.Service;
import com.danniu.netkit.Box;
import com.danniu.netkit.IBox;

/**
 * Created by dantezhu on 14-10-20.
 */
public class MyFerryDelegate implements Delegate {
    public void onOpen(Service service) {
        Log.v(G.LOG_TAG, "onOpen");
        Box box = new Box();
        box.cmd = 1;

        G.service.send(box);
    }

    // 主要是打印log
    public void onSend(Service service, IBox ibox) {
        Box box = (Box)ibox;
        Log.v(G.LOG_TAG, "onSend: box: " + box);
    }

    public void onRecv(Service service, IBox ibox) {
        Box box = (Box)ibox;
        Log.v(G.LOG_TAG, "onRecv: box: " + box);

        SimpleEvent event = new SimpleEvent(1);
        G.eventBus.post(event);
    }
    public void onClose(Service service) {
        Log.v(G.LOG_TAG, "onClose");
    }
    public void onError(Service service, int code) {
        Log.v(G.LOG_TAG, "onError code: " + code);
    }

    public IBox allocBox() {
        return new Box();
    }

}
