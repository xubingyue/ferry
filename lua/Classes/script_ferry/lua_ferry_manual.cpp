//
// Created by dantezhu on 14-10-23.
//

#include "lua_ferry_manual.hpp"
#include "tolua_fix.h"
#include "LuaBasicConversions.h"
#include "CCLuaValue.h"
#include "CCLuaEngine.h"
#include "ScriptFerry.h"
#include "ScriptBox.h"

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
    if (2 == argc) {
#if COCOS2D_DEBUG >= 1
        // 2 代表第一个参数
        if (!toluafix_isfunction(tolua_S,2,"LUA_FUNCTION",0,&tolua_err) ||
                !tolua_isusertype(tolua_S,3, "cc.Ref", 0, &tolua_err)
                )
        {
            goto tolua_lerror;
        }
#endif
        LUA_FUNCTION handler =  toluafix_ref_function(tolua_S,2,0);
        cocos2d::Ref* target = (cocos2d::Ref*)tolua_tousertype(tolua_S,3,0);
        int tolua_ret = self->scriptAddEventCallback(handler, target);
        tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        return 1;
    }

    CCLOG("'scriptAddEventCallback' has wrong number of arguments: %d, was expecting %d\n", argc, 2);
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

    if (1 == argc) {
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
        self->send(box);
        tolua_pushnumber(tolua_S,(lua_Number)0);
        return 1;
    }
    else if (4 == argc) {
#if COCOS2D_DEBUG >= 1
        if (
                !tolua_isusertype(tolua_S,2, "netkit.IBox", 0, &tolua_err) ||
                !toluafix_isfunction(tolua_S,3,"LUA_FUNCTION",0,&tolua_err) ||
                !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
                !tolua_isusertype(tolua_S,5, "cc.Ref", 0, &tolua_err)
                )
        {
            goto tolua_lerror;
        }
#endif
        netkit::IBox* box = (netkit::IBox*)tolua_tousertype(tolua_S,2,0);
        LUA_FUNCTION handler =  toluafix_ref_function(tolua_S,3,0);
        float timeout = (float)  tolua_tonumber(tolua_S,4,0);
        cocos2d::Ref* target = (cocos2d::Ref*)tolua_tousertype(tolua_S,5,0);
        int tolua_ret = self->scriptSend(box, handler, timeout, target);
        tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        return 1;
    }

    CCLOG("'scriptSend' has wrong number of arguments: %d, was expecting %d or %d\n", argc, 1, 4);
    return 0;

#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'scriptSend'.",&tolua_err);
    return 0;
#endif
}

static int tolua_ferry_ScriptBox_getBody(lua_State *tolua_S)
{
    if (NULL == tolua_S)
        return 0;

    int argc = 0;
    ferry::ScriptBox * self = nullptr;

#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
    if (!tolua_isusertype(tolua_S,1,"ferry.ScriptBox",0,&tolua_err)) goto tolua_lerror;
#endif

    self = static_cast<ferry::ScriptBox *>(tolua_tousertype(tolua_S,1,0));

#if COCOS2D_DEBUG >= 1
    if (nullptr == self) {
        tolua_error(tolua_S,"invalid 'self' in function 'tolua_ferry_ScriptBox_getBody'\n", NULL);
        return 0;
    }
#endif

    argc = lua_gettop(tolua_S) - 1;
    if (0 == argc) {
        // 减少内存拷贝
        const std::string& strBody = self->getStringBody();
        // 二进制
        lua_pushlstring(tolua_S, strBody.c_str(), strBody.size());
        return 1;
    }

    CCLOG("'getBody' has wrong number of arguments: %d, was expecting %d\n", argc, 0);
    return 0;

#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getBody'.",&tolua_err);
    return 0;
#endif
}

static int tolua_ferry_ScriptBox_setBody(lua_State* tolua_S)
{
    if (NULL == tolua_S)
        return 0;

    int argc = 0;
    ferry::ScriptBox * self = nullptr;

#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
    if (!tolua_isusertype(tolua_S,1,"ferry.ScriptBox",0,&tolua_err)) goto tolua_lerror;
#endif

    self = static_cast<ferry::ScriptBox *>(tolua_tousertype(tolua_S,1,0));

#if COCOS2D_DEBUG >= 1
    if (nullptr == self) {
        tolua_error(tolua_S,"invalid 'self' in function 'tolua_ferry_ScriptBox_setBody'\n", NULL);
        return 0;
    }
#endif

    argc = lua_gettop(tolua_S) - 1;

    if (1 == argc) {
#if COCOS2D_DEBUG >= 1
        if (
                !tolua_isstring(tolua_S,2, 0, &tolua_err)
                )
        {
            goto tolua_lerror;
        }
#endif
        size_t size = 0;
        const char* pBody = lua_tolstring(tolua_S, 2, &size);
        // null也调用，代表body设置为空
        self->setBody(pBody, size);

        // 没有返回值
        return 0;
    }

    CCLOG("'setBody' has wrong number of arguments: %d, was expecting %d\n", argc, 1);
    return 0;

#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'setBody'.",&tolua_err);
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

static void extendScriptBox(lua_State* tolua_S)
{
    lua_pushstring(tolua_S,"ferry.ScriptBox");
    lua_rawget(tolua_S,LUA_REGISTRYINDEX);
    if (lua_istable(tolua_S,-1))
    {
        lua_pushstring(tolua_S,"getBody");
        lua_pushcfunction(tolua_S, tolua_ferry_ScriptBox_getBody);
        lua_rawset(tolua_S,-3);
        lua_pushstring(tolua_S,"setBody");
        lua_pushcfunction(tolua_S, tolua_ferry_ScriptBox_setBody);
        lua_rawset(tolua_S,-3);
    }
    lua_pop(tolua_S, 1);
}

int register_all_ferry_manual(lua_State* tolua_S)
{
    if (NULL == tolua_S)
        return 0;

    extendScriptFerry(tolua_S);
    extendScriptBox(tolua_S);

    return 0;
}
