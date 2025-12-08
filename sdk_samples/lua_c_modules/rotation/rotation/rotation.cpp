//
// rotation.cpp: sample implementation of fsmapper pluagin as a Lua C module
//   Author: Hiroshi Murayama <opiopan@gmail.com>
//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mapperplugin_luac.h>
extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}

#include <memory>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <numbers>
#include <cmath>
#include <limits>

#pragma comment(lib, "fsmappercore.lib")

//============================================================================================
// DLL entry point
//============================================================================================
BOOL APIENTRY DllMain(HMODULE, DWORD reason_for_call, LPVOID){
    switch (reason_for_call){
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//============================================================================================
// Emitter object that simulates rotating coordinates emission
//============================================================================================
class emitter{
public:
    enum class mode{cw, ccw, stop,};
    using list = std::list<emitter>;

protected:
    inline static const char *userdata_name = "rotation.emitter";
    inline static list emitters;

    std::mutex mutex;
    std::condition_variable cv;
    bool should_be_stopped{false};
    std::thread worker;
    FSMAPPER_LUAC_CTX fsmapper{nullptr};
    FSMAPPER_LUAC_ASYNC_SOURCE async_source{nullptr};
    FSMAPPER_EVENT_ID event_id;
    double rpm;
    double radius;
    double angle{0};
    mode direction{mode::stop};
    using clock = std::chrono::steady_clock;
    clock::time_point prev_time;
    list::iterator self;

    class emitter_ref{
        list::iterator ref;
    public:
        emitter_ref(list::iterator ref):ref(ref){}
        ~emitter_ref() = default;
        emitter* operator -> (){return ref.operator->();}
    };


public:
    emitter(FSMAPPER_LUAC_CTX fsmapper, FSMAPPER_EVENT_ID event_id, double rpm, double side_length) :
        fsmapper(fsmapper), event_id(event_id), rpm(rpm), radius(side_length / 2.){
        worker = std::thread([this]{
            std::unique_lock lock{mutex};
            while(true){
                if (direction == mode::stop){
                    cv.wait(lock);
                }else{
                    cv.wait_for(lock, std::chrono::milliseconds(50));
                }
                if (should_be_stopped){
                    break;
                }
                if (direction != mode::stop){
                    // Notify fsmapper that an asynchronous event has occurred
                    // The event ID and event value are returned at the moment event_provider() is invoked.
                    //
                    // Note:
                    // Unlocking the mutex here is very important.
                    // fsmapper_luac_* functions may block due to inter-thread synchronization inside fsmapper.
                    // Therefore, they must NOT be called while holding (i.e., blocking) the thread of any
                    // Lua C function that is invoked as part of a running Lua script (i.e., functions that
                    // receive a lua_State* L). Doing so can lead to a classic deadlock scenario.
                    lock.unlock();
                    fsmapper_luac_async_source_signal(async_source);
                    lock.lock();
                }
            }

        });
    }

    ~emitter(){
        {
            std::lock_guard lock{mutex};
            should_be_stopped = true;
            cv.notify_all();
        }
        worker.join();
    }

    //--------------------------------------------------------------------------------------------
    // Lua C function to create emitter user data
    //--------------------------------------------------------------------------------------------
    static int create_emitter(lua_State* L){
        // Check arguments
        FSMAPPER_EVENT_ID event_id = luaL_checkinteger(L, 1);
        double rpm = luaL_checknumber(L, 2);
        double side_length = luaL_checknumber(L, 3);

        // Create emitter object
        auto fsmapper = fsmapper_luac_open_ctx(L, userdata_name);
        emitters.emplace_back(fsmapper, event_id, rpm, side_length);
        auto object = std::prev(emitters.end());
        object->self = object;

        // Create Lua user data that holds a reference to the emitter object
        auto udata_ptr = lua_newuserdata(L, sizeof(object));
        auto udata = lua_gettop(L);
        new(udata_ptr) emitter_ref(object);

        // Register the metatable for Lua user data (only on the first call)
        if (luaL_newmetatable(L, userdata_name)){
            auto meta_table = lua_gettop(L);
            // Set __gc field
            lua_pushcfunction(L, gc);
            lua_setfield(L, meta_table, "__gc");
            // Set __index field
            static const luaL_Reg methods[] = {
                {"start_cw", start_cw},
                {"start_ccw", start_ccw},
                {"stop", stop},
                {nullptr, nullptr},
            };
            lua_newtable(L);
            luaL_setfuncs(L, methods, 0);
            lua_setfield(L, meta_table, "__index");
        }

        // Set metatable into new user data
        lua_setmetatable(L, udata);

        // Create asynchronous event source and register event provider function that will be called in the Lua scripting thread
        object->async_source = fsmapper_luac_create_async_source(fsmapper, L, event_provider, udata);

        // Retern one value, note that the stack top is a user object that reference to the emitter object
        return 1;
    }

protected:
    //--------------------------------------------------------------------------------------------
    // Lua C functions for members of the emitter user data
    //--------------------------------------------------------------------------------------------
    static int start_cw(lua_State* L){
        auto udata = reinterpret_cast<emitter_ref*>(luaL_checkudata(L, 1, userdata_name));
        auto& self = *udata->operator ->();
        std::lock_guard lock{self.mutex};
        self.direction = mode::cw;
        self.prev_time = clock::now();
        self.cv.notify_all();
        return 0;
    }

    static int start_ccw(lua_State* L){
        auto udata = reinterpret_cast<emitter_ref*>(luaL_checkudata(L, 1, userdata_name));
        auto& self = *udata->operator ->();
        std::lock_guard lock{self.mutex};
        self.direction = mode::ccw;
        self.prev_time = clock::now();
        self.cv.notify_all();
        return 0;
    }

    static int stop(lua_State* L){
        auto udata = reinterpret_cast<emitter_ref*>(luaL_checkudata(L, 1, userdata_name));
        auto& self = *udata->operator ->();
        std::lock_guard lock{self.mutex};
        self.direction = mode::stop;
        self.cv.notify_all();
        return 0;
    }

    static int gc(lua_State* L){
        auto udata = reinterpret_cast<emitter_ref*>(luaL_checkudata(L, 1, userdata_name));
        fsmapper_luac_release_async_source((*udata)->async_source, L);
        (*udata)->emitters.erase((*udata)->self);
        (*udata).~emitter_ref();
        return 0;
    }

    //--------------------------------------------------------------------------------------------
    // Lua C function to return asynchronous events to fsmapper
    //    This function is invoked on fsmapper's Lua script thread after fsmapper_luac_create_async_source()
    //    notifies fsmapper that an asynchronous event has occurred.
    //--------------------------------------------------------------------------------------------
    static int event_provider(lua_State* L){
        auto udata = reinterpret_cast<emitter_ref*>(luaL_checkudata(L, 1, userdata_name));
        auto& self = *udata->operator ->();
        std::lock_guard lock{self.mutex};

        // update angle and calculate coordinates
        auto now = clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - self.prev_time).count() / 60000.;
        self.angle += (self.direction == mode::cw ? 1. : -1) * 2. * std::numbers::pi * self.rpm * dt;
        auto x = std::cos(self.angle) * self.radius + self.radius;
        auto y = std::sin(self.angle) * self.radius + self.radius;
        self.prev_time = now;
        
        // Return event-id and 2-dimensional coordinates as a table:
        //   return event_id, {x=x, y=y}
        lua_pushinteger(L, self.event_id);
        lua_newtable(L);
        auto rtable = lua_gettop(L);
        lua_pushnumber(L, x);
        lua_setfield(L, rtable, "x");
        lua_pushnumber(L, y);
        lua_setfield(L, rtable, "y");

        return 2;
    }
};

//============================================================================================
// Lua C module entry point
//============================================================================================
extern "C" __declspec(dllexport) int luaopen_rotation(lua_State* L){
    static const luaL_Reg module_funcs[] = {
        {"emitter", emitter::create_emitter},
        {nullptr, nullptr},
    };

    luaL_newlib(L, module_funcs);
    return 1;
}