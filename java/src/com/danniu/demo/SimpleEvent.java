package com.danniu.demo;

import android.os.Bundle;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by zny2008 on 13-12-22.
 * 一个基本得event类
 */
public class SimpleEvent {
    public int what;
    public Bundle bundle;
    // 需要的时候才用
    public Map<String, Object> extra;

    public SimpleEvent() {
        this(0);
    }

    public SimpleEvent(int what) {
        this(what, null);
    }

    public SimpleEvent(int what, Bundle bundle) {
        this(what, bundle, null);
    }

    public SimpleEvent(int what, Bundle bundle, Map<String, Object> extra) {
        this.what = what;
        this.bundle = bundle == null ? new Bundle() : bundle;
        this.extra = extra == null ? new HashMap<String, Object>() : extra;
    }

    public String toString() {
        return "" + getClass().getSimpleName() + ", what: " + what + ", bundle: " + bundle + ", extra: " + extra;
    }
}
