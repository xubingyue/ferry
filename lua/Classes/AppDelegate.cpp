// 在文件中记得引入

#include "script_ferry/lua_ferry_auto.hpp"
#include "script_ferry/lua_ferry_manual.hpp"

    // ...

    LuaStack* stack = engine->getLuaStack();
    stack->setXXTEAKeyAndSign("2dxLua", strlen("2dxLua"), "XXTEA", strlen("XXTEA"));
    
    //register custom function
    //LuaStack* stack = engine->getLuaStack();
    //register_custom_function(stack->getLuaState());

// ADD-BEGIN by dantezhu in 2014-10-24 17:02:48
    // lua_gettop将看到为堆栈长度为0
    // 将_G压入，务必调用，否则无法使用
    lua_getglobal(stack->getLuaState(), "_G");

    register_all_ferry(stack->getLuaState());
    register_all_ferry_manual(stack->getLuaState());

    // 清空堆栈，在最后调用
    lua_settop(stack->getLuaState(), 0);
// ADD-END


#if (COCOS2D_DEBUG>0)
    if (startRuntime())
        return true;
#endif

    // ....
