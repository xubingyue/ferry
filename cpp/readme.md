### 一. 使用方法

#### I. 基于事件总线

使用Service + eventbus

##### 缺点:
    lua无法使用，因为lua重写的onEvent无法在c++中找到.

##### 注意:
1. Delegate拿到的数据并不一定在主线程，如果需要回调到主线程，请配合 [eventbus](https://github.com/dantezhu/eventbus) 使用
2. Service内部默认是不会释放send/recv的box的，这个释放操作需要delegate负责。onSend/onRecv/onError中的box都需要释放

#### II. 基于回调

使用Ferry::getInstance()

##### 回调类型
    * 发送回调

        只接收 box.sn > 0 的响应
        能够接收的事件类型:
            
            EVENT_SEND
            EVENT_RECV
            EVENT_ERROR
                CODE: ERROR_SEND
            EVENT_TIMEOUT
                CODE: ERROR_RECV

    * 事件回调

        只接收 box.sn = 0 的响应
        能够接收的事件类型:

            EVENT_OPEN
            EVENT_SEND
            EVENT_RECV
            EVENT_ERROR
                CODE: ERROR_OPEN
            EVENT_CLOSE
            EVENT_TIMEOUT
                CODE: ERROR_OPEN

##### 注意:
    * 所有使用send、或者addEventCallback的类，析构函数里面务必调用如下代码，防止崩溃

        delCallbacksForTarget

### 二. 设计说明

1. Box、Event，均没有使用cocos2d的内存管理，原因autorelease函数不是线程安全。

### 三. IDE 配置

1. windows
    * pthread 下载: ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-9-1-release.zip

            在vs中配置对应的include和lib，如果报dll找不到，将dll放到Debug.win32目录即可

2. mac
    * 为了正确找到头文件，需要在header目录添加:
        * appcode

            在 User Header Search Path 中添加

            "$(SRCROOT)/../Classes"

### 四. 最佳实践
1. send函数调用后，box依然可以确保存在。

    调用方可以在send后安全获取box，不会出现box被删除的情况。
    因为box会在主线程回调onSend后被删除。
    即使触发onDisconnect也不会清楚历史events，所以不会删除历史box。
    同时box.sn会被修改为最新的sn，所以调用方可以用来标识最后一次发送的sn。

### 五. 注意
1. stop始终找不到太好的方法，原因如下:

    1. stop不关闭thread，只是标记为不运行

        这种在正常使用没有问题，但是在c++的对象释放时，会释放不了

    2. stop 直接杀死进程（cpp用信号，java用stop）

        这种可能会导致死锁的问题，即锁没有被释放

    综上所述，最好不要反复的调用start和stop，在正常开发中也确实没有必要调用

2. service内部client的线程安全

    这个一直没有太好的方法解决，主要因为client.read是阻塞的，无法加锁

    不过出现的概率很低，只有在判断client != null 而 client又被置为null时会出现
