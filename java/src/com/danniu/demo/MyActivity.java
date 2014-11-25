package com.danniu.demo;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import com.danniu.ferry.R;

public class MyActivity extends Activity {
    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        G.eventBus.register(this);

        G.service.init(new MyFerryDelegate(), "192.168.1.110", 7777);
        G.service.start();
    }

    @Override
    protected void onDestroy() {
        G.eventBus.unregister(this);

        super.onDestroy();
    }

    public void onEventMainThread(SimpleEvent event) {
        Log.v(G.LOG_TAG, "event: " + event);
    }
}
