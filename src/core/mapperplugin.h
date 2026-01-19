//
// mapperplugin.h
//
// Copyright 2021 Hiroshi Murayama <opiopan@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <mapperplugin_types.h>

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================================
// Funcitons to interact with fsmapper
//============================================================================================
typedef struct FSMAPPERCTX* FSMAPPER_HANDLE;
__declspec(dllexport) void fsmapper_putLog(FSMAPPER_HANDLE mapper, FSMAPPER_LOG_TYPE type, const char* msg);
__declspec(dllexport) void fsmapper_abort(FSMAPPER_HANDLE mapper);
__declspec(dllexport) void fsmapper_setContext(FSMAPPER_HANDLE mapper, void *context);
__declspec(dllexport) void *fsmapper_getContext(FSMAPPER_HANDLE mapper);

typedef struct FSMDEVICECTX* FSMDEVICE;
__declspec(dllexport) void fsmapper_setContextForDevice(FSMAPPER_HANDLE mapper, FSMDEVICE device, void* context);
__declspec(dllexport) void* fsmapper_getContextForDevice(FSMAPPER_HANDLE mapper, FSMDEVICE device);
__declspec(dllexport) void fsmapper_raiseEvent(FSMAPPER_HANDLE mapper, FSMDEVICE device, int index, int value);

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

__declspec(dllexport) LVTYPE luav_getType(LUAVALUE lv);
__declspec(dllexport) bool luav_isNull(LUAVALUE lv);
__declspec(dllexport) bool luav_asBool(LUAVALUE lv);
__declspec(dllexport) int64_t luav_asInt(LUAVALUE lv);
__declspec(dllexport) double luav_asDouble(LUAVALUE lv);
__declspec(dllexport) const char* luav_asString(LUAVALUE lv);
__declspec(dllexport) LUAVALUE luav_getItemWithKey(LUAVALUE lv, const char* key);
__declspec(dllexport) LUAVALUE luav_getItemWithIndex(LUAVALUE lv, size_t index);

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

typedef bool (*FSMDEV_INIT)(FSMAPPER_HANDLE mapper);
typedef bool (*FSMDEV_TERM)(FSMAPPER_HANDLE mapper);
typedef bool (*FSMDEV_OPEN)(FSMAPPER_HANDLE mapper, FSMDEVICE device, LUAVALUE identifier, LUAVALUE options);
typedef bool (*FSMDEV_START)(FSMAPPER_HANDLE mapper, FSMDEVICE device);
typedef bool (*FSMDEV_CLOSE)(FSMAPPER_HANDLE mapper, FSMDEVICE device);
typedef size_t (*FSMDEV_GET_UNIT_NUM)(FSMAPPER_HANDLE mapper, FSMDEVICE device);
typedef bool (*FSMDEV_GET_UNIT_DEF)(FSMAPPER_HANDLE mapper, FSMDEVICE device, size_t index, FSMDEVUNITDEF *def);
typedef bool (*FSMDEV_SEND_UNIT_VALUE)(FSMAPPER_HANDLE mapper, FSMDEVICE device, size_t index, int value);

typedef struct {
    const char* name;
    const char* description;
    FSMDEV_INIT init;
    FSMDEV_TERM term;
    FSMDEV_OPEN open;
    FSMDEV_START start;
    FSMDEV_CLOSE close;
    FSMDEV_GET_UNIT_NUM getUnitNum;
    FSMDEV_GET_UNIT_DEF getUnitDef;
    FSMDEV_SEND_UNIT_VALUE sendUnitValue;
} MAPPER_PLUGIN_DEVICE_OPS;

// fsmapper will determine that plugin module has a device plugin capability
// if a module exports this function.
__declspec(dllexport) const MAPPER_PLUGIN_DEVICE_OPS* getMapperPluginDeviceOps();

#ifdef __cplusplus
}
#endif
