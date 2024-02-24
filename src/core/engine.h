//
// engine.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <sol/sol.hpp>
#include "mappercore.h"
#include "mappercore_inner.h"
#include "option.h"
#include "event.h"
#include "action.h"

class DeviceManager;
class DeviceModifier;
class DeviceModifierManager;
class SimHostManager;
class ViewPortManager;
class vJoyManager;

using MapperException = std::runtime_error;

class MapperEngine {
public:
    typedef std::function<void(MAPPER_EVENT event, int64_t data)> Callback;
    typedef std::function<void(MCONSOLE_MESSAGE_TYPE, const std::string &)> Logger;
    enum class Status{
        init,
        prepare_to_run,
        running,
        stop,
        error,
    };
    using CLOCK = std::chrono::steady_clock;
    using TIME_POINT = CLOCK::time_point;
    using MILLISEC = std::chrono::milliseconds;
    static constexpr auto UPDATED_DEVICES = 0x1;
    static constexpr auto UPDATED_MAPPINGS = 0x2;
    static constexpr auto UPDATED_VIEWPORTS = 0x4;
    static constexpr auto UPDATED_VJOY = 0x8;
    static constexpr auto UPDATED_VIEWPORTS_STATUS = 0x10;
    static constexpr auto UPDATED_READY_TO_CAPTURE = 0x20;
    static constexpr auto UPDATED_LOST_CAPTURED_WINDOW = 0x40;

protected : 
    std::mutex mutex;
    Status status;
    bool callback_is_inhibited = false;
    Callback callback;
    Logger logger;
    MAPPER_LOGMODE logmode;
    TIME_POINT now = CLOCK::now();
    MapperOption options;

    struct {
        std::string scriptPath;
        std::unique_ptr<sol::state> lua_ptr;
        std::unique_ptr<DeviceManager> deviceManager;
        std::unique_ptr<ViewPortManager> viewportManager;
        std::unique_ptr<SimHostManager> simhostManager;
        std::unique_ptr<vJoyManager> vjoyManager;

        sol::state& lua(){return *lua_ptr;};
        bool should_gc = true;

        uint32_t updated_flags = 0;
    }scripting;

    class DeferredAction{
    protected:
        std::shared_ptr<Action> action;
        Event event;
    public:
        DeferredAction() = delete;
        DeferredAction(const DeferredAction&) = delete;
        DeferredAction(DeferredAction&& src): action(src.action), event(std::move(src.event)){};
        DeferredAction(std::shared_ptr<Action> action, const Event& event): action(action), event(event){};
        ~DeferredAction() = default;
        std::shared_ptr<Action> get_action(){return action;};
        Event& get_event(){return event;};
    };

    struct {
        std::condition_variable cv;
        uint64_t idCounter;
        std::map<uint64_t, std::string> names;
        std::queue< std::unique_ptr<Event> > queue;
        std::map<TIME_POINT, DeferredAction> deferred_actions;
        bool need_update_viewports = false;
        bool touch_event_occurred = false;
        TIME_POINT view_updated_time;
    }event;

    std::unique_ptr<EventActionMap> mapping[2];

public:
    MapperEngine(Callback callback, Logger logger);
    virtual ~MapperEngine();

    bool run(std::string&& scriptPath);
    bool stop();
    bool abort();

    const MapperOption& getOptions() const{return options;}
    bool setOption(MAPPER_OPTION type, const char* value){
        std::lock_guard lock(mutex);
        return options.set_value(type, value);
    }
    bool setOption(MAPPER_OPTION type, int64_t value){
        std::lock_guard lock(mutex);
        return options.set_value(type, value);
    }

    void inhibitCallback(){
        std::lock_guard lock(mutex);
        callback_is_inhibited = true;
    }

    void setLogmode(MAPPER_LOGMODE mode){
        std::lock_guard lock(mutex);
        logmode = mode;
    }

    Status getStatus(){
        std::lock_guard lock(mutex);
        return status;
    };

    void putLog(MCONSOLE_MESSAGE_TYPE mtype, const std::string& msg){
        if (!callback_is_inhibited){
            if (mtype == MCONSOLE_DEBUG && !(logmode & MAPPER_LOG_DEBUG)){
                return;
            }
            logger(mtype, msg);
        }
    };

    void recommend_gc(){
        std::lock_guard lock(mutex);
        scripting.should_gc = true;
    };

    uint64_t registerEvent(std::string&& name);
    void unregisterEvent(uint64_t evid);
    const char* getEventName(uint64_t evid) const;
    void sendEvent(Event&& event);
    void sendHostEvent(MAPPER_EVENT event, int64_t data);

    void invokeActionIn(std::shared_ptr<Action> action, const Event& event, MILLISEC millisec);

    void notifyUpdate(uint32_t flag){
        std::lock_guard lock(mutex);
        scripting.updated_flags |= flag;
        event.cv.notify_all();
    }

    void notifyUpdateWithNoLock(uint32_t flag){
        scripting.updated_flags |= flag;
        event.cv.notify_all();
    }

    void invokeViewportsUpdate(){
        // this function must be called from the event-action mapping loop (thread)
        // and this flag (need_update_viewports) is refered by only that thread
        // so no need to guard by mutex lock and no need to notify condition variable also
        event.need_update_viewports = true;
    }

    void notifyTouchEvent(){
        std::lock_guard lock(mutex);
        event.touch_event_occurred = true;
        event.cv.notify_all();
    }

    sol::state& getLuaState(){return scripting.lua();};

    SimHostManager* getSimHostManager(){return scripting.simhostManager.get();}

    // interfaces for host program
    std::vector<CapturedWindowInfo> get_captured_window_list();
    std::vector<std::string> get_captured_window_titles(uint32_t cwid);
    struct DeviceInfo {
        std::string device_name;
        const char* class_name;
        DeviceInfo(const char* dname, const char* cname) : device_name(dname), class_name(cname){}
    };
    std::vector<DeviceInfo> get_device_list();
    std::vector<ViewportInfo> get_viewport_list();
    void register_captured_window(uint32_t cwid, HWND hWnd);
    void unregister_captured_window(uint32_t cwid);
    bool enable_viewports();
    bool disable_viewports();
    MAPPINGS_STAT get_mapping_stat();
    
protected:
    void initScriptingEnv();
    std::unique_ptr<Event> receiveEvent();
    Action* findAction(uint64_t evid);

    void setMapping(const char* function_name, int level, const sol::object& mapdef);
    void addMapping(const char* function_name, int level, const sol::object& mapdef);
};

template <typename T>
inline auto lua_c_interface(MapperEngine& engine, const char* function_name, T function){
    try{
        return function();
    }catch (MapperException& e){
        std::ostringstream os;
        os << "[" << function_name << "()] " << e.what();
        engine.putLog(MCONSOLE_ERROR, os.str().c_str());
        std::ostringstream os2;
        os2 << e.what() << std::endl << "----------------------------------------------------";
        throw std::runtime_error(os2.str());
    }
}
