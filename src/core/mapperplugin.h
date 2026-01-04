//
// mapperplugin.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <mapperplugin_types.h>

#if defined(_WIN64) || defined(_WIN32)
#   define DLLEXPORT __declspec(dllexport)
#else
#   define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================================
// Funcitons to interuct with fsmapper
//============================================================================================
typedef struct FSMAPPERCTX* FSMAPPER_HANDLE;
DLLEXPORT void fsmapper_putLog(FSMAPPER_HANDLE mapper, FSMAPPER_LOG_TYPE type, const char* msg);
DLLEXPORT void fsmapper_abort(FSMAPPER_HANDLE mapper);
DLLEXPORT void fsmapper_setContext(FSMAPPER_HANDLE mapper, void *context);
DLLEXPORT void *fsmapper_getContext(FSMAPPER_HANDLE mapper);

typedef struct FSMDEVICECTX* FSMDEVICE;
DLLEXPORT void fsmapper_setContextForDevice(FSMAPPER_HANDLE mapper, FSMDEVICE device, void* context);
DLLEXPORT void* fsmapper_getContextForDevice(FSMAPPER_HANDLE mapper, FSMDEVICE device);
DLLEXPORT void fsmapper_raiseEvent(FSMAPPER_HANDLE mapper, FSMDEVICE device, int index, int value);

//============================================================================================
// LUA value accessor
//============================================================================================
typedef struct LUAVALUECTX* LUAVALUE;

typedef enum {
    LV_NULL,
    LV_BOOL,
    LV_NUMBER,
    LV_STRING,
    LV_TABLE,
    LV_OTHERS,
}LVTYPE;

DLLEXPORT LVTYPE luav_getType(LUAVALUE lv);
DLLEXPORT bool luav_isNull(LUAVALUE lv);
DLLEXPORT bool luav_asBool(LUAVALUE lv);
DLLEXPORT int64_t luav_asInt(LUAVALUE lv);
DLLEXPORT double luav_asDouble(LUAVALUE lv);
DLLEXPORT const char* luav_asString(LUAVALUE lv);
DLLEXPORT LUAVALUE luav_getItemWithKey(LUAVALUE lv, const char* key);
DLLEXPORT LUAVALUE luav_getItemWithIndex(LUAVALUE lv, size_t index);

//============================================================================================
// Device plugin functions to export
// These functions are impremented by plugin module and are called by fsmapper
//============================================================================================
typedef enum {
    FSMDU_DIR_INPUT,
    FSMDU_DIR_OUTPUT,
}FSMDEVUNIT_DIRECTION;

typedef enum {
    FSMDU_TYPE_ABSOLUTE,
    FSMDU_TYPE_RELATIVE,
    FSMDU_TYPE_BINARY,
}FSMDEVUNIT_VALTYPE;

typedef struct {
    const char* name;
    FSMDEVUNIT_DIRECTION direction;
    FSMDEVUNIT_VALTYPE type;
    int maxValue;
    int minValue;
}FSMDEVUNITDEF;

typedef struct {
    const char* name;
    const char* description;
    bool (*init)(FSMAPPER_HANDLE mapper);
    bool (*term)(FSMAPPER_HANDLE mapper);
    bool (*open)(FSMAPPER_HANDLE mapper, FSMDEVICE device, LUAVALUE identifier, LUAVALUE options);
    bool (*start)(FSMAPPER_HANDLE mapper, FSMDEVICE device);
    bool (*close)(FSMAPPER_HANDLE mapper, FSMDEVICE device);
    size_t (*getUnitNum)(FSMAPPER_HANDLE mapper, FSMDEVICE device);
    bool (*getUnitDef)(FSMAPPER_HANDLE mapper, FSMDEVICE device, size_t index, FSMDEVUNITDEF* def);
    bool (*sendUnitValue)(FSMAPPER_HANDLE mapper, FSMDEVICE device, size_t index, int value);
} MAPPER_PLUGIN_DEVICE_OPS;

// fsmapper will determine that plugin module has a device plugin capability
// if a module exports this function.
DLLEXPORT MAPPER_PLUGIN_DEVICE_OPS* getMapperPluginDeviceOps();

#ifdef __cplusplus
}
#endif
