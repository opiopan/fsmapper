//
// mapperplugin_luac.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <mapperplugin_types.h>

#if defined(_WIN64) || defined(_WIN32)
    #define DLLEXPORT __declspec(dllexport)
#else
    #define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

struct _FSMAPPER_LUAC_CTX;
typedef struct _FSMAPPER_LUAC_CTX* FSMAPPER_LUAC_CTX;

DLLEXPORT FSMAPPER_LUAC_CTX fsmapper_luac_open_ctx(lua_State* L, const char* mod_name);
DLLEXPORT void fsmapper_luac_release_ctx(FSMAPPER_LUAC_CTX ctx);
DLLEXPORT void fsmapper_luac_putLog(FSMAPPER_LUAC_CTX ctx, FSMAPPER_LOG_TYPE type, const char* msg);
DLLEXPORT void fsmapper_luac_send_event(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid);
DLLEXPORT void fsmapper_luac_send_event_int(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid, long long data);
DLLEXPORT void fsmapper_luac_send_event_float(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid, double data);
DLLEXPORT void fsmapper_luac_send_event_str(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid, const char* data);

struct _FSMAPPER_LUAC_ASYNC_SOURCE;
typedef struct _FSMAPPER_LUAC_ASYNC_SOURCE* FSMAPPER_LUAC_ASYNC_SOURCE;
DLLEXPORT FSMAPPER_LUAC_ASYNC_SOURCE fsmapper_luac_create_async_source(FSMAPPER_LUAC_CTX ctx, lua_CFunction event_provider);
DLLEXPORT void fsmapper_luac_release_async_source(FSMAPPER_LUAC_ASYNC_SOURCE source);
DLLEXPORT void fsmapper_luac_async_source_signal(FSMAPPER_LUAC_ASYNC_SOURCE source);

#ifdef __cplusplus
}
#endif
