### 一. 使用方法

#### I. 原始用法
直接使用service，基于onEvent的事件总线方式

缺点是，lua无法使用，因为lua重写的onEvent无法在c++中找到.

简单来说，在使用时，只要注意:

1. onEvent中的event，只能在这个函数中使用，函数结束即会被释放。其中的box也是如此。不要在匿名函数中使用.
2. 调用send的box，必须使用new创建，send之后，就不再使用
3. 调用pushEvent的event，使用new创建，且pushEvent之后就不再使用
4. delegate的函数是可能在任何线程调用的，所以如果想要全部转化成主线程，请使用eventbus做转化
5. eventbus是线程安全的

#### II. 基于回调

即使用Ferry::getInstance()

全部基于回调，lua也可以方便使用。

最重要的一点是，所有使用send、或者addEventCallback的类，析构函数里面务必调用:

    delCallbacksForTarget

防止出现崩溃

### 二. 设计说明

1. Box、BaseEvent，均没有使用cocos2d的内存管理，因为很大的原因是autorelease函数貌似不是线程安全。
2. app调用send传给ferry的box，在ferry发送完之后会自动delete.
3. service调用回调函数传入的box，在回调函数处理结束后会自动释放