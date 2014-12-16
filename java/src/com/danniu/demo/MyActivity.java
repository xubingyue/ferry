package com.danniu.demo;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import com.danniu.ferry.Ferry;
import com.danniu.ferry.R;
import com.danniu.netkit.Box;
import com.danniu.netkit.IBox;

public class MyActivity extends Activity {
    public static final String LOG_TAG = "ferry_test";

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        regEventCallback();

        Ferry.getInstance().init("192.168.1.115", 7777);
        Ferry.getInstance().start();
    }

    @Override
    protected void onDestroy() {
        Ferry.getInstance().delCallbacksForTarget(this);

        super.onDestroy();
    }

    private void regEventCallback() {
        Ferry.getInstance().addEventCallback(new Ferry.CallbackListener() {
            public void onOpen() {
                Box box = new Box();
                box.version = 100;
                box.flag = 99;
                box.cmd = 1;
                box.body = new String("I love you").getBytes();

                Ferry.getInstance().send(box, new Ferry.CallbackListener() {
                    public void onRecv(IBox ibox) {
                        Log.d(LOG_TAG, "recv: " + ibox);
                    }
                }, 5, this);
            }

            public void onRecv(IBox ibox) {
                Log.d(LOG_TAG, "recv: " + ibox);
            }

            public void onClose() {
                Ferry.getInstance().connect();
            }

        }, this, "ok");
    }

}
