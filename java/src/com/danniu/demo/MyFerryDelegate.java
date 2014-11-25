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
        box.version = 100;
        box.flag = 99;
        box.cmd = 1;
        box.body = new String("I love you").getBytes();

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
        G.service.connect();
    }
    public void onError(Service service, int code, IBox ibox) {
        Log.v(G.LOG_TAG, "onError code: " + code);
    }

    public IBox createBox() {
        return new Box();
    }

}
