package com.danniu.ferry;

import com.danniu.netkit.IBox;

/**
 * Created by dantezhu on 14-10-19.
 */
public interface Delegate {
    public void onOpen(Service service);
    // 主要是打印log
    public void onSend(Service service, IBox ibox);
    public void onRecv(Service service, IBox ibox);
    public void onClose(Service service);
    public void onError(Service service, int code);

    public IBox createBox();
}
