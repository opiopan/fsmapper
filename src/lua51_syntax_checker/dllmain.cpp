// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <string>
#include "lua51checker.hpp"


extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}

struct lua51_checker{
    lua_State* L;
    std::string last_error;

    lua51_checker(){
        L = luaL_newstate();
    }

    ~lua51_checker(){
        lua_close(L);
    }

    bool check_syntax(const char* chunk){
        auto rc = luaL_loadstring(L, chunk) == 0;
        if (!rc){
            last_error = lua_tostring(L, -1);
        }
        lua_pop(L, 1);
        return rc;
    }

    const char* get_last_error(){
        return last_error.c_str();
    }
};

extern "C" __declspec(dllexport) LUA51CHECKER lua51checker_open(){
    return new lua51_checker;
}

extern "C" __declspec(dllexport) void lua51checker_close(LUA51CHECKER checker){
    delete reinterpret_cast<lua51_checker*>(checker);
}

extern "C" __declspec(dllexport) bool lua51checker_check_syntax(LUA51CHECKER checker, const char *chunk){
    return reinterpret_cast<lua51_checker*>(checker)->check_syntax(chunk);
}

extern "C" __declspec(dllexport) const char *lua51checker_get_last_error(LUA51CHECKER checker){
    return reinterpret_cast<lua51_checker*>(checker)->get_last_error();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
