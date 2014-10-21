package com.danniu.demo;

import com.danniu.ferry.Service;
import de.greenrobot.event.EventBus;

/**
 * Created by dantezhu on 14-10-20.
 */
public class G {
    public static EventBus eventBus = new EventBus();
    public static Service service = new Service();
    public final static String LOG_TAG = "demo";
}
