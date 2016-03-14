package cn.vimer.ferry_demo;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import cn.vimer.ferry.Ferry;
import cn.vimer.netkit.Box;
import cn.vimer.netkit.IBox;

public class MyActivity extends Activity {
    public static final String LOG_TAG_EVENT = "ferry_event";
    public static final String LOG_TAG_RSP = "ferry_rsp";

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        regEventCallback();

        Ferry.getInstance().init("192.168.0.108", 7777);
        Ferry.getInstance().start();
    }

    @Override
    protected void onDestroy() {
        Ferry.getInstance().delCallbacksForTarget(this);

        super.onDestroy();
    }

    private void regEventCallback() {
        Ferry.getInstance().addEventCallback(new Ferry.CallbackListener() {
            @Override
            public void onOpen() {
                Log.d(LOG_TAG_EVENT, "onOpen");

                Box box = new Box();
                box.version = 100;
                box.flag = 99;
                box.cmd = 1;
                box.body = new String("I love you").getBytes();

                Ferry.getInstance().send(box, new Ferry.CallbackListener() {
                    @Override
                    public void onSend(IBox ibox) {
                        Log.d(LOG_TAG_RSP, String.format("onSend, box: %s", ibox));
                    }

                    @Override
                    public void onRecv(IBox ibox) {
                        Log.d(LOG_TAG_RSP, String.format("onRecv, box: %s", ibox));
                    }

                    @Override
                    public void onError(int code, IBox ibox) {
                        Log.d(LOG_TAG_RSP, String.format("onError, code: %s, box: %s", code, ibox));
                    }

                    @Override
                    public void onTimeout(int code, IBox ibox) {
                        Log.d(LOG_TAG_RSP, "onTimeout");
                    }
                }, 5, this);
            }

            @Override
            public void onRecv(IBox ibox) {
                Log.d(LOG_TAG_EVENT, String.format("onRecv, box: %s", ibox));
            }

            @Override
            public void onClose() {
                Log.d(LOG_TAG_EVENT, "onClose");
                Ferry.getInstance().connect();
            }

            @Override
            public void onError(int code, IBox ibox) {
                Log.d(LOG_TAG_EVENT, String.format("onError, code: %s, box: %s", code, ibox));
            }

        }, this, "ok");
    }

}
