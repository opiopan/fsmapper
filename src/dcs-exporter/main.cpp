//
// main.cpp: Utility functions for DCS exporter Lua script
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
//  Note: 
//    This file contains code for a C-module that can be loaded into the DCS World Lua environment.
//    Therefore, the Lua version targeted by this file is 5.1.
//

#include <windows.h>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}

#include "struct.hpp"

//============================================================================================
// Lua C module entry point
//============================================================================================
static luaL_Reg module[]{
    {"struct", lua_struct::create_struct},
    {nullptr, nullptr},
};

extern "C" __declspec(dllexport) int luaopen_fsmapper_utils(lua_State* L){
    // register a metatable for "struct"
    lua_struct::register_meta_table(L);

    // register module
    luaL_register(L, "fsmapper_utils", module);
    return 1;
}

//============================================================================================
// DLL entry point
//============================================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
