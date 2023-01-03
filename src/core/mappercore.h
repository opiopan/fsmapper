//
// mappercore.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(_WIN64) || defined(_WIN32)
#   include <windows.h>
#   define DLLEXPORT __declspec(dllexport)
#else
#   define HWND int
#   define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t MAPPER_LOGMODE;
#define MAPPER_LOG_DEBUG 0x1
#define MAPPER_LOG_EVENT 0x2

typedef enum mapper_event{
    MEV_START_MAPPING,
    MEV_STOP_MAPPING,
    MEV_CHANGE_SIMCONNECTION = 10000,
    MEV_CHANGE_AIRCRAFT,
    MEV_CHANGE_DEVICES,
    MEV_CHANGE_MAPPINGS,
    MEV_CHANGE_VJOY,
    MEV_READY_TO_CAPTURE_WINDOW,
    MEV_LOST_CAPTURED_WINDOW,
    MEV_CHANGE_VIEWPORTS,
    MEV_START_VIEWPORTS,
    MEV_STOP_VIEWPORTS,
    MEV_RESET_VIEWPORTS,
}MAPPER_EVENT;

typedef uint32_t MAPPER_SIM_CONNECTION;
#define MAPPER_SIM_NONE   0x0
#define MAPPER_SIM_FS2020 0x1
#define MAPPER_SIM_DCS    0x2

typedef MAPPER_SIM_CONNECTION MEVDATA_CHANGE_SIMCONNECTION;
typedef const char* MEVDATA_CHANGE_AIRCRAFT;

typedef enum {
    MCONSOLE_ERROR,
    MCONSOLE_WARNING,
    MCONSOLE_INFO,
    MCONSOLE_MESSAGE,
    MCONSOLE_DEBUG,
    MCONSOLE_EVENT,
}MCONSOLE_MESSAGE_TYPE;

typedef struct{
    uint32_t cwid;
    const char *name;
    const char *description;
    bool isCaptured;
}CAPTURED_WINDOW_DEF;

typedef struct{
    int num_primery;
    int num_secondary;
    int num_for_viewports;
    int num_for_views;
}MAPPINGS_STAT;

typedef struct{
    const char* viewport_name;
    int32_t viewid;
    const char* view_name;
}VIEWPORT_DEF;

typedef enum{
    MOPT_PRE_RUN_SCRIPT,    // string
    MOPT_RENDERING_METHOD,  // integer
}MAPPER_OPTION;

typedef enum{
    MOPT_RENDERING_METHOD_CPU,
    MOPT_RENDERING_METHOD_GPU,
}MAPPER_OPTION_RENDERING_METHOD;

struct MapperContext;
typedef struct MapperContext* MapperHandle;

typedef bool (*MAPPER_CALLBACK_FUNC)(MapperHandle mapper, MAPPER_EVENT event, int64_t data);
typedef bool (*MAPPER_CONSOLE_HANDLER)(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char*msg, size_t len);
typedef bool (*MAPPER_ENUM_DEVICE_FUNC)(MapperHandle mapper, void* context, const char* devtype, const char* devname);
typedef bool (*MAPPER_ENUM_CAPUTURED_WINDOW)(MapperHandle mapper, void* context, CAPTURED_WINDOW_DEF* cwdef);
typedef bool (*MAPPER_ENUM_VIEWPORT_FUNC)(MapperHandle mapper, void* context, VIEWPORT_DEF* vpdef);

DLLEXPORT MapperHandle mapper_init(MAPPER_CALLBACK_FUNC callback, MAPPER_CONSOLE_HANDLER logger, void *hostContext);
DLLEXPORT bool mapper_terminate(MapperHandle handle);

DLLEXPORT bool mapper_set_option_integer(MapperHandle handle, MAPPER_OPTION option, int64_t value);
DLLEXPORT bool mapper_set_option_string(MapperHandle handle, MAPPER_OPTION option, const char* value);

DLLEXPORT bool mapper_run(MapperHandle handle, const char* scriptPath);
DLLEXPORT bool mapper_stop(MapperHandle handle);

DLLEXPORT bool mapper_setLogMode(MapperHandle handle, MAPPER_LOGMODE logmode);

DLLEXPORT void* mapper_getHostContext(MapperHandle handle);
DLLEXPORT MAPPER_SIM_CONNECTION mapper_getSimConnection(MapperHandle handle);
DLLEXPORT const char* mapper_getAircraftName(MapperHandle handle);
DLLEXPORT MAPPINGS_STAT mapper_getMappingsStat(MapperHandle handle);

DLLEXPORT bool mapper_enumDevices(MapperHandle handle, MAPPER_ENUM_DEVICE_FUNC func, void* context);
DLLEXPORT bool mapper_enumCapturedWindows(MapperHandle handle, MAPPER_ENUM_CAPUTURED_WINDOW func, void* context);
DLLEXPORT bool mapper_enumViewport(MapperHandle handle, MAPPER_ENUM_VIEWPORT_FUNC func, void* context);
DLLEXPORT bool mapper_captureWindow(MapperHandle handle, uint32_t cwid, HWND hWnd);
DLLEXPORT bool mapper_releaseWindw(MapperHandle handle, uint32_t cwid);

DLLEXPORT bool mapper_startViewPort(MapperHandle handle);
DLLEXPORT bool mapper_stopViewPort(MapperHandle handle);

//
// Stateless tool functions
//
DLLEXPORT HWND mapper_tools_PickWindow(HWND app_wnd, const char* name);

//
// functions provided as workaround of WindowsApp SDK 1.2 issues
//
DLLEXPORT HRESULT mapper_getAppDataPath(PWSTR *ppszPath);
DLLEXPORT HRESULT mapper_getLocalDataPath(PWSTR *ppszPath);

#ifdef __cplusplus
}
#endif