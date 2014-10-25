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
            EVENT_TIMEOUT

    * 事件回调

        只接收 box.sn = 0 的响应
        能够接收的事件类型:

            EVENT_OPEN
            EVENT_SEND
            EVENT_RECV
            EVENT_ERROR
            EVENT_CLOSE
            EVENT_TIMEOUT

##### 注意:
    * 所有使用send、或者addEventCallback的类，析构函数里面务必调用如下代码，防止崩溃

        delCallbacksForTarget

### 二. 设计说明

1. Box、Event，均没有使用cocos2d的内存管理，原因autorelease函数不是线程安全。

### 三. 平台兼容

1. windows
    * pthread 下载: ftp://sourceware.org/pub/pthreads-win32

        在vs中配置对应的include和lib，如果报dll找不到，将dll放到Debug.win32目录即可
