### Ferry-Lua 使用方法

1. 新建cocos2dx-lua工程

2. 复制如下目录至新工程的Classes目录

    netkit
        cpp/netkit 
    ferry
        cpp/Classes/ferry 
        lua/Classes/script_ferry
        lua/Classes/auto

3. 修改 AppDelegate.cpp，参考 ferry/lua/Classes/AppDelegate.cpp

4. 复制相关的lua文件至心工程的src目录
    ferry
        lua/script/*

5. 修改main.lua，参考
    ferry
        lua/demo/main.lua
