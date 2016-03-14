package cn.vimer.ferry;

import cn.vimer.netkit.IBox;

/**
 * Created by dantezhu on 14-10-19.
 */
public interface Delegate {
    public void onOpen(Service service);
    public void onSend(Service service, IBox ibox);
    public void onRecv(Service service, IBox ibox);
    public void onClose(Service service);
    public void onError(Service service, int code, IBox ibox);
    public void onTimeout(Service service);

    public IBox createBox();
}
