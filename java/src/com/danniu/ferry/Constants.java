package com.danniu.ferry;

/**
 * Created by dantezhu on 14-10-25.
 */
public class Constants {
    public final static int MSG_QUEUE_TO_SERVER_MAX_SIZE = 100;
    public final static int TRY_CONNECT_INTERVAL = 1;

    // 错误代码
    public final static int ERROR_OPEN = 1;
    public final static int ERROR_SEND = 2;

    // 事件类型
    public static final int EVENT_OPEN = 1;
    public static final int EVENT_SEND = 2;
    public static final int EVENT_RECV = 3;
    public static final int EVENT_CLOSE = 4;
    public static final int EVENT_ERROR = 5;
    public static final int EVENT_TIMEOUT = 6;

    // 超时检测间隔
    public static final long TIMEOUT_CHECK_INTERVAL_MS = 500;

    // LOG_TAG
    public static final String LOG_TAG = "ferry";
}
