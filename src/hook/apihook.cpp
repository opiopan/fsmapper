//
// apihook.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "apihook.h"
#include <imagehlp.h>
#pragma comment(lib,"imagehlp.lib")

void* swapApiEntryPoint(HMODULE hModule, const char* func_name, void* hook_func){
    ULONG ulSize;
    auto pImgDesc = 
        reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(ImageDirectoryEntryToData(
            hModule, true, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize));
    auto dwBase = reinterpret_cast<DWORD_PTR>(hModule);
    for (; pImgDesc->Name; pImgDesc++) {
        auto szModuleName = reinterpret_cast<const char*>(dwBase + pImgDesc->Name);
        // retrieve thunk information
        auto pFirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(dwBase + pImgDesc->FirstThunk);
        auto pOrgFirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(dwBase + pImgDesc->OriginalFirstThunk);
        
        // find function to swap
        for (; pFirstThunk->u1.Function; pFirstThunk++, pOrgFirstThunk++) {
            if (IMAGE_SNAP_BY_ORDINAL(pOrgFirstThunk->u1.Ordinal)){
                continue;
            }
            auto pImportName = 
                reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(dwBase + pOrgFirstThunk->u1.AddressOfData);
            if (_stricmp(pImportName->Name, func_name) == 0){
                // unprotect memory accessing
                DWORD dwOldProtect;
                if (!VirtualProtect(&pFirstThunk->u1.Function, sizeof(pFirstThunk->u1.Function), PAGE_READWRITE, &dwOldProtect))
                    return NULL;

                // swap function address
                auto pOrgFunc = reinterpret_cast<void*>(pFirstThunk->u1.Function);
                WriteProcessMemory(
                    GetCurrentProcess(), &pFirstThunk->u1.Function, &hook_func, 
                    sizeof(pFirstThunk->u1.Function), NULL);
                pFirstThunk->u1.Function = reinterpret_cast<DWORD_PTR>(hook_func);

                // restore memory protection mode
                VirtualProtect(&pFirstThunk->u1.Function, sizeof(pFirstThunk->u1.Function), dwOldProtect, &dwOldProtect);
                return pOrgFunc;
            }
        }
    }
    return NULL;
}
