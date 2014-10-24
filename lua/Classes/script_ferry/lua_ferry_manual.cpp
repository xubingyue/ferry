//
// Created by dantezhu on 14-10-23.
//

#include "lua_ferry_manual.h"
#include "tolua_fix.h"
#include "LuaBasicConversions.h"
#include "CCLuaValue.h"
#include "CCLuaEngine.h"
#include "ScriptFerry.h"

static int tolua_ferry_ScriptFerry_scriptAddEventCallback(lua_State *tolua_S)
{
    if (NULL == tolua_S)
        return 0;

    int argc = 0;
    ferry::ScriptFerry * self = nullptr;

#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
    if (!tolua_isusertype(tolua_S,1,"ferry.ScriptFerry",0,&tolua_err)) goto tolua_lerror;
#endif

    self = static_cast<ferry::ScriptFerry *>(tolua_tousertype(tolua_S,1,0));

#if COCOS2D_DEBUG >= 1
    if (nullptr == self) {
        tolua_error(tolua_S,"invalid 'self' in function 'tolua_ferry_ScriptFerry_scriptAddEventCallback'\n", NULL);
        return 0;
    }
#endif

    argc = lua_gettop(tolua_S) - 1;
    if (1 == argc) {
#if COCOS2D_DEBUG >= 1
        if (!toluafix_isfunction(tolua_S,2,"LUA_FUNCTION",0,&tolua_err))
        {
            goto tolua_lerror;
        }
#endif
        LUA_FUNCTION handler =  toluafix_ref_function(tolua_S,2,0);
        int tolua_ret = self->scriptAddEventCallback(handler);
        tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        return 1;
    }

    CCLOG("'scriptAddEventCallback' has wrong number of arguments: %d, was expecting %d\n", argc, 1);
    return 0;

#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'scriptAddEventCallback'.",&tolua_err);
    return 0;
#endif
}

static int tolua_ferry_ScriptFerry_scriptSend(lua_State* tolua_S)
{
    if (NULL == tolua_S)
        return 0;

    int argc = 0;
    ferry::ScriptFerry * self = nullptr;

#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
    if (!tolua_isusertype(tolua_S,1,"ferry.ScriptFerry",0,&tolua_err)) goto tolua_lerror;
#endif

    self = static_cast<ferry::ScriptFerry *>(tolua_tousertype(tolua_S,1,0));

#if COCOS2D_DEBUG >= 1
    if (nullptr == self) {
        tolua_error(tolua_S,"invalid 'self' in function 'tolua_ferry_ScriptFerry_scriptSend'\n", NULL);
        return 0;
    }
#endif

    argc = lua_gettop(tolua_S) - 1;
    if (3 == argc) {
#if COCOS2D_DEBUG >= 1
        if (
                !tolua_isusertype(tolua_S,2, "netkit.IBox", 0, &tolua_err) ||
                !toluafix_isfunction(tolua_S,3,"LUA_FUNCTION",0,&tolua_err) ||
                !tolua_isnumber(tolua_S,4,0,&tolua_err)
                )
        {
            goto tolua_lerror;
        }
#endif
        netkit::IBox* box = (netkit::IBox*)tolua_tousertype(tolua_S,2,0);
        LUA_FUNCTION handler =  toluafix_ref_function(tolua_S,3,0);
        float timeout = (float)  tolua_tonumber(tolua_S,4,0);
        int tolua_ret = self->scriptSend(box, handler, timeout);
        tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        return 1;
    }
    else if (1 == argc) {
#if COCOS2D_DEBUG >= 1
        // 只有一个参数也是可以的
        if (
                !tolua_isusertype(tolua_S,2, "netkit.IBox", 0, &tolua_err)
                )
        {
            goto tolua_lerror;
        }
#endif
        netkit::IBox* box = (netkit::IBox*)tolua_tousertype(tolua_S,2,0);
        int tolua_ret = self->send(box);
        tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        return 1;
    }

    CCLOG("'scriptSend' has wrong number of arguments: %d, was expecting %d\n", argc, 3);
    return 0;

#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'scriptSend'.",&tolua_err);
    return 0;
#endif
}

static void extendScriptFerry(lua_State* tolua_S)
{
    lua_pushstring(tolua_S,"ferry.ScriptFerry");
    lua_rawget(tolua_S,LUA_REGISTRYINDEX);
    if (lua_istable(tolua_S,-1))
    {
        lua_pushstring(tolua_S,"addEventCallback");
        lua_pushcfunction(tolua_S, tolua_ferry_ScriptFerry_scriptAddEventCallback);
        lua_rawset(tolua_S,-3);
        lua_pushstring(tolua_S,"send");
        lua_pushcfunction(tolua_S, tolua_ferry_ScriptFerry_scriptSend);
        lua_rawset(tolua_S,-3);
    }
    lua_pop(tolua_S, 1);
}

int register_all_ferry_manual(lua_State* tolua_S)
{
    if (NULL == tolua_S)
        return 0;

    extendScriptFerry(tolua_S);
    return 0;
}
