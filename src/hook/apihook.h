//
// apihook.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>
#include <string>

void* swapApiEntryPoint(HMODULE hModule, const char* func_name, void* hook_func);

template <typename TFunction>
class HookedApi;

template <typename TResult, typename ...TArgs>
class HookedApi<TResult (TArgs...)>{
public:
    using function_ptr = TResult (*)(TArgs...);

protected:
    HMODULE target_module = nullptr;
    std::string func_name;
    function_ptr origin_function = nullptr;
    function_ptr hook_function = nullptr;

public:
    HookedApi() = default;
    HookedApi(const HookedApi&) = delete;
    HookedApi(HookedApi&&) = delete;
    ~HookedApi(){
        if (origin_function){
            swapApiEntryPoint(target_module, func_name.c_str(), origin_function);
        }
    }

    bool setHook(const char* module_name, const char* func_name, function_ptr hook){
        if (origin_function){
            return false;
        }
        target_module = ::GetModuleHandleA(module_name);
        if (target_module){
            this->func_name = func_name;
            this->hook_function = hook;
            origin_function = 
                reinterpret_cast<function_ptr>(swapApiEntryPoint(
                    target_module, this->func_name.c_str(), this->hook_function));
        }
        return origin_function;
    }

    bool removeHook(){
        if (origin_function){
            auto rc = swapApiEntryPoint(target_module, func_name.c_str(), origin_function);
            target_module = nullptr;
            origin_function = nullptr;
            hook_function = nullptr;
            return rc;
        }
        return true;
    }

    function_ptr get_origin(){return origin_function;};
    function_ptr get_hook(){return hook_function;};
    operator function_ptr (){return origin_function;};
    operator bool (){return origin_function;};
};