### 安装说明
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

### 使用说明
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
