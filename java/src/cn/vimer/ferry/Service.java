package cn.vimer.ferry;

import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.concurrent.ArrayBlockingQueue;

import cn.vimer.netkit.IBox;
import cn.vimer.netkit.Stream;


/**
 * Created by dantezhu on 14-10-19.
 */
public class Service {

    private ArrayBlockingQueue<IBox> msgQueueToServer = new ArrayBlockingQueue<IBox>(Constants.MSG_QUEUE_TO_SERVER_MAX_SIZE);

    private Delegate delegate;
    private String host;
    private int port;
    private boolean running = false;
    // 最后一次活跃时间，包括onOpen和onRecv。onClose时要清零
    private long lastActiveTimeMills = 0;

    private boolean shouldConnect = false;

    private Stream client;
    private int tryConnectInterval = Constants.TRY_CONNECT_INTERVAL;
    private int connectTimeout = Constants.CONNECT_TIMEOUT;

    private Thread recvThread = null;
    private Thread sendThread = null;

    public Service() {
    }

    public int init(Delegate delegate, String host, int port) {
        this.delegate = delegate;
        this.host = host;
        this.port = port;
        return 0;
    }

    public void start() {
        if (running) {
            return;
        }

        running = true;

        connect();
        startThreads();
    }

    public void stop() {
        if (!running) {
            return;
        }
        running = false;

        stopThreads();
        closeConn();
        clearMsgQueueToServer();
    }

    public void connect() {
        shouldConnect = true;
    }

    public void disconnect() {
        if (client != null) {
            client.shutdown(2);
        }
    }

    public boolean isConnected() {
        return client != null && !client.isClosed();
    }

    public boolean isRunning() {
        return running;
    }

    public long getLastActiveTimeMills() {
        return lastActiveTimeMills;
    }

    public void send(IBox ibox) {
        boolean succ = msgQueueToServer.offer(ibox);
        if (!succ) {
            onError(Constants.ERROR_SEND, ibox);
        }
    }

    public void setTryConnectInterval(int interval) {
        tryConnectInterval = interval;
    }

    public void setConnectTimeout(int timeout) {
        connectTimeout = timeout;
    }

    private void closeConn() {

        if (client != null) {
            try{
                client.close();
            }
            catch (Exception e) {
                FLog.e("e: " + e);
            }
            finally {
                client = null;
            }

        }
    }

    private void startThreads() {
        recvThread = new Thread(new Runnable() {
            @Override
            public void run() {
                recvMsgFromServer();
            }
        });
        recvThread.start();

        sendThread = new Thread(new Runnable() {
            @Override
            public void run() {
                sendMsgToServer();
            }
        });
        sendThread.start();
    }

    private void stopThreads() {
        if (sendThread != null && sendThread.isAlive()) {
            // stop 在android上会抛异常
            sendThread.interrupt();
            sendThread = null;
        }

        if (recvThread != null && recvThread.isAlive()) {
            recvThread.interrupt();
            recvThread = null;
        }
    }

    private void recvMsgFromServer() {
        while (running) {
            if (!isConnected()) {
                if (shouldConnect) {
                    connectToServer();
                }

                if (!isConnected()) {
                    // 毫秒
                    try{
                        Thread.sleep(tryConnectInterval * 1000);
                    }
                    catch (Exception e) {
                        FLog.e("e: " + e);
                    }

                    continue;
                }
            }

            IBox box = delegate.createBox();

            try{
                boolean succ = client.read(box);
                if (succ) {
                    onRecvMsgFromServer(box);
                }
                else {
                    // 先自己也关闭掉
                    closeConn();
                    onConnClose();
                }
            }
            catch (Exception e) {
                FLog.e("e: " + e);
                closeConn();
                onConnClose();
            }
        }
    }

    private void sendMsgToServer() {

        while (running) {
            IBox box = null;

            try {
                box = msgQueueToServer.take();
                if (box != null){
                    if (isConnected()) {
                        client.write(box);
                        onSendMsgToServer(box);
                    }
                    else {
                        onError(Constants.ERROR_SEND, box);
                    }
                }
            }
            catch (Exception e) {
                FLog.e("e: " + e);
                onError(Constants.ERROR_SEND, box);
            }
        }
    }

    private void connectToServer() {
        closeConn();

        Socket socket;

        try {
            socket = new Socket();
            InetSocketAddress address = new InetSocketAddress(host, port);
            socket.connect(address, connectTimeout * 1000);
        }
        catch (SocketTimeoutException e) {
            FLog.e("e: " + e);
            onTimeout();
            return;
        }
        catch (Exception e) {
            FLog.e("e: " + e);
            onError(Constants.ERROR_OPEN, null);
            return;
        }
        // 第二步创建stream对象
        if (client != null) {
            client.setSocket(socket);
        }
        else {
            client = new Stream(socket);
        }

        onConnOpen();
    }

    private void onConnClose() {
        lastActiveTimeMills = 0;
        shouldConnect = false;
        delegate.onClose(this);
    }

    private void onConnOpen() {
        lastActiveTimeMills = System.currentTimeMillis();
        delegate.onOpen(this);
    }

    private void onError(int code, IBox ibox) {
        delegate.onError(this, code, ibox);
    }

    private void onTimeout() {
        delegate.onTimeout(this);
    }

    private void onSendMsgToServer(IBox ibox) {
        delegate.onSend(this, ibox);
    }

    private void onRecvMsgFromServer(IBox ibox) {
        lastActiveTimeMills = System.currentTimeMillis();
        delegate.onRecv(this, ibox);
    }

    // 想在析构的时候调用，但是好像调用不到。。
    private void clearMsgQueueToServer() {
        while (true) {
            IBox box = msgQueueToServer.poll();
            if (box == null) {
                break;
            }
            else {
                onError(Constants.ERROR_SEND, box);
            }
        }
    }
}
