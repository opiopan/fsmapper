//
// dllmain.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <stdint.h>
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>

BOOL APIENTRY DllMain(
    HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    static uint64_t token;
    static Gdiplus::GdiplusStartupInput input;
    static Gdiplus::GdiplusStartupOutput output;

    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
        return Gdiplus::GdiplusStartup(&token, &input, &output) == Gdiplus::Ok;
    case DLL_PROCESS_DETACH:
        Gdiplus::GdiplusShutdown(token);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

