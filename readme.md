# Ferry - 客户端网络层

### 一. 支持语言
* cpp(cocos2dx-cpp)
* lua(cocos2dx-lua)
* java

具体使用说明请进入对应语言的子目录

### 二. 依赖项目
* [netkit](https://github.com/dantezhu/netkit)

### 三. 服务器端

* python
    * [haven](https://github.com/dantezhu/haven)
    * [melon](https://github.com/dantezhu/melon)

### 四. 注意
1. stop始终找不到太好的方法，原因如下:

    1. stop不关闭thread，只是标记为不运行

        这种在正常使用没有问题，但是在c++的对象释放时，会释放不了

    2. stop 直接杀死进程（cpp用信号，java用stop）

        这种可能会导致死锁的问题，即锁没有被释放

    综上所述，最好不要反复的调用start和stop，在正常开发中也确实没有必要调用

2. service内部client的线程安全

    这个一直没有太好的方法解决，主要因为client.read是阻塞的，无法加锁

    不过出现的概率很低，只有在判断client != null 而 client又被置为null时会出现

### 五. 示例项目
1. cocos2dx + ferry + protobuf(lua)

    [cocos_lua_net_demo](https://github.com/dantezhu/cocos_lua_net_demo)
