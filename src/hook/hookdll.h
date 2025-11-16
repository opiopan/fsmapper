//
// hookdll.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DLLEXPORT __declspec(dllexport)

#define MAX_CAPTURED_WINDOW 32

#define CAPTURE_OPT_HIDE_SYSTEM_REGION  1
#define CAPTURE_OPT_MODIFY_TOUCH        2

typedef struct  _TOUCH_CONFIG{
    uint32_t down_delay;
    uint32_t up_delay;
    uint32_t start_delay;
    bool     double_tap_on_drag;
    uint32_t dead_zone_for_drag_start;
}TOUCH_CONFIG;

typedef void (*WINDOW_CLOSE_CALLBACK)(HWND hWnd, void* context);

DLLEXPORT bool hookdll_startGlobalHook(WINDOW_CLOSE_CALLBACK callback, void* context);
DLLEXPORT bool hookdll_stopGlobalHook();

DLLEXPORT bool hookdll_capture(HWND hWnd, DWORD option);
DLLEXPORT bool hookdll_uncapture(HWND hWnd);
DLLEXPORT bool hookdll_changeWindowAtrribute(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, bool show);

DLLEXPORT void hookdll_setWindowForRecovery(HWND hwnd, int type);
DLLEXPORT void hookdll_setTouchParameters(const TOUCH_CONFIG* config);

#ifdef __cplusplus
}
#endif
