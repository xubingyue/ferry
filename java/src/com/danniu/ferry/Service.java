package com.danniu.ferry;

import android.util.Log;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.danniu.netkit.IBox;
import com.danniu.netkit.Stream;


/**
 * Created by dantezhu on 14-10-19.
 */
public class Service {

    public final static String LOG_TAG = "ferry";


    private ArrayBlockingQueue<IBox> msgQueueToServer = new ArrayBlockingQueue<IBox>(Constants.MSG_QUEUE_TO_SERVER_MAX_SIZE);

    private Delegate delegate;
    private String host;
    private int port;
    private boolean running = false;

    private boolean shouldConnect = false;

    private Stream client;
    private int tryConnectInterval = Constants.TRY_CONNECT_INTERVAL;

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
        // TODO stop

        closeConn();
    }

    public void connect() {
        shouldConnect = true;
    }

    public void closeConn() {
        // 和c++一样
        // clearMsgQueues();

        if (client != null) {
            try{
                client.close();
            }
            catch (Exception e) {
                Log.e(LOG_TAG, "e: " + e);
            }
            finally {
                client = null;
            }

        }
    }

    public boolean isConnected() {
        return client == null ? false : !client.isClosed();
    }

    public boolean isRunning() {
        return running;
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
                        Log.e(LOG_TAG, "e: " + e);
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
                Log.e(LOG_TAG, "e: " + e);
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
                Log.e(LOG_TAG, "e: " + e);
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
            socket.connect(address);
        }
        catch (Exception e) {
            Log.e(LOG_TAG, "e: " + e);
            onError(Constants.ERROR_OPEN, null);
            return;
        }
        // 第二步创建stream对象
        client = new Stream(socket);

        onConnOpen();
    }

    private void clearMsgQueues() {
        msgQueueToServer.clear();
    }

    private void onConnClose() {
        shouldConnect = false;
        delegate.onClose(this);
    }

    private void onConnOpen() {
        delegate.onOpen(this);
    }
    private void onError(int code, IBox ibox) {
        delegate.onError(this, code, ibox);
    }

    private void onSendMsgToServer(IBox ibox) {
        delegate.onSend(this, ibox);
    }

    private void onRecvMsgFromServer(IBox ibox) {
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
