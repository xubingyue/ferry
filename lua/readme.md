### 一. 安装说明
1. 新建cocos2dx-lua工程
2. 复制如下目录至新工程的Classes目录

        netkit
            cpp/netkit 
        ferry
            cpp/Classes/ferry 
            lua/Classes/script_ferry
            lua/Classes/auto
3. 修改 AppDelegate.cpp，参考 

        ferry/lua/Classes/AppDelegate.cpp
4. 复制相关的lua文件至心工程的src目录

        ferry
            lua/script/*
5. 修改main.lua，参考

        ferry
            lua/demo/main.lua

### 二. 使用说明
1. 语法演示 

        ferry.ScriptFerry:getInstance():start()
2. 删除回调

    与cpp版的删除回调不同

    lua的send和addEventCallback都会返回一个entryID，调用放可以调用 
    
        delRspCallback(entryID)
        delEventCallback(entryID)

    来实现删除

3. 事件定义
    
    由于tolua无法导出枚举，所以手工写了时间定义在FerryConstants.lua 中

### 三. tolua编译说明
1. 将tolua目录下的ferry.ini 和 genbindings_ferry.py 放到

        project/frameworks/cocos2d-x/tools/tolua

2. 将conversions.xml中的ns_map修改同步到

        project/frameworks/cocos2d-x/tools/bindings-generator/targets/lua/conversions.xml

3. 将ferry 和 netkit 复制到 Classes目录

4. 添加 project的 User Header Search Path

        "$(SRCROOT)/../Classes"
    
5. 添加cocos2d_lua_bindings中的User Header Search Path

        $(SRCROOT)/../../../../../runtime-src/Classes/

6. 到tolua目录，执行

        python genbindings_ferry.py
