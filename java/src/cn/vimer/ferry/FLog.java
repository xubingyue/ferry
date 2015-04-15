package cn.vimer.ferry;

import android.util.Log;

public class FLog {
    /**
     * 对外static调用
     */
    private static FLog instance = getXLogger("ferry");

    public static FLog getXLogger(String tag) {
        return new FLog(tag);
    }

    public static void v(String msg){
        instance.verbose(msg);
    }

    public static void d(String msg){
        instance.debug(msg);
    }

    public static void i(String msg){
        instance.info(msg);
    }

    public static void e(String msg){
        instance.error(msg);
    }


    /**
     * log tag
     */
    private String tag = null;//application name

    public FLog(String tag) {
        this.tag = tag;
    }

    /**
     * 获取函数名称
     */
    private String getFunctionName() {
        StackTraceElement[] sts = Thread.currentThread().getStackTrace();

        if (sts == null) {
            return null;
        }

        for (StackTraceElement st : sts) {
            if (st.isNativeMethod()) {
                continue;
            }

            if (st.getClassName().equals(Thread.class.getName())) {
                continue;
            }

            if (st.getClassName().equals(this.getClass().getName())) {
                continue;
            }

            return "[" + Thread.currentThread().getName() + "(" + Thread.currentThread().getId()
                + ") " + st.getFileName() + ":" + st.getLineNumber() + " " + st.getMethodName() + "]";
        }

        return null;
    }

    private String createMessage(String msg) {
        String functionName = getFunctionName();
        String message = (functionName == null ? msg : (functionName + ": " + msg));
        return message;
    }

    public void verbose(String msg){
        String message = createMessage(msg);
        Log.v(tag, message);
    }

    public void debug(String msg){
        String message = createMessage(msg);
        Log.d(tag, message);
    }

    public void info(String msg){
        String message = createMessage(msg);
        Log.i(tag, message);
    }

    public void error(String msg){
        String message = createMessage(msg);
        Log.e(tag, message);
    }
}
