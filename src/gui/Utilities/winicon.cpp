//
// winicon.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "winicon.hpp"
#include <shellapi.h>
#include <psapi.h>

#pragma comment(lib, "Psapi.lib")

namespace utils {
    HICON get_window_icon(HWND hwnd){
        HICON icon{nullptr};

        // Retrieve an icon associated with the specified window
        icon = reinterpret_cast<HICON>(SendMessage(hwnd, WM_GETICON, ICON_BIG, 0));
        if (icon == nullptr){
            icon = reinterpret_cast<HICON>(SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0));
        }
        if (icon == nullptr){
            icon = reinterpret_cast<HICON>(SendMessage(hwnd, WM_GETICON, ICON_SMALL2, 0));
        }

        // Try to retrieve an icon from the window class
        if (icon == nullptr){
            icon = reinterpret_cast<HICON>(GetClassLongPtr(hwnd, GCLP_HICON));
        }
        if (icon == nullptr){
            icon = reinterpret_cast<HICON>(GetClassLongPtr(hwnd, GCLP_HICONSM));
        }

        // Otherwise retreave the representative icon from the EXE file
        if (icon == nullptr){
            DWORD process_id{0};
            GetWindowThreadProcessId(hwnd, &process_id);
            HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
            if (process){
                wchar_t exePath[MAX_PATH];
                if (GetModuleFileNameExW(process, nullptr, exePath, MAX_PATH)){
                    SHFILEINFO shFileInfo;
                    if (SHGetFileInfo(exePath, 0, &shFileInfo, sizeof(shFileInfo), SHGFI_ICON | SHGFI_LARGEICON)){
                        icon = shFileInfo.hIcon;
                    }
                }
                CloseHandle(process);
            }
        }

        return icon;
    }
}