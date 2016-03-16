### 一. 安装说明
1. 新建cocos2dx-lua工程
2. 复制如下目录至新工程的Classes目录

        netkit/cpp/netkit 
        ferry/cpp/Classes/ferry 
        ferry/lua/Classes/script_ferry
3. 修改 AppDelegate.cpp，参考 

        ferry/lua/Classes/AppDelegate.cpp
4. 复制相关的lua文件至新工程的src目录

        ferry/lua/script/*
5. 修改main.lua，参考

        ferry/lua/demo/main.lua

6. windows下需要自己配置pthread，并在运行时将相关dll放到运行目录下

### 二. 使用说明
1. 语法演示 

        ferry.ScriptFerry:getInstance():start()
2. 删除回调

    与cpp版的删除回调不同

    lua的send和addEventCallback都会返回一个entryID，调用方可以调用 
    
        delRspCallback(entryID)
        delEventCallback(entryID)

    来实现删除

3. 事件定义
    
    由于tolua无法导出枚举，所以手工写了时间定义在FerryConstants.lua 中

### 三. tolua++编译说明
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

7. 生成hpp和cpp文件到script_ferry中
    
        runtime-src/Classes/script_ferry/lua_ferry_auto.hpp
        runtime-src/Classes/script_ferry/lua_ferry_auto.cpp

8. 搞定

### 四. 说明
1. 版本库中目前自动生成的cpp文件使用环境为:
    
        mac 10.9 ~ 10.11
        cocos2dx-3.2
        ndk-r9d ~ r10c
        python 2.7 (pip install cheetah pyyaml)


### 五. 注意
1. 在mac 10.11下，tolua会报错


    LibclangError: dlopen(libclang.dylib, 6): image not found. To provide a path to libclang use Config.set_library_path() or Config.set_library_file().


解决方案为:

    ./frameworks/cocos2d-x/tools/bindings-generator/clang/cindex.py 第 3395 行 改为 ： library = cdll.LoadLibrary("../bindings-generator/libclang/" + self.get_filename())
