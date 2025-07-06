//
// engine.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <filesystem>
#include "hookdll.h"
#include "engine.h"
#include "device.h"
#include "simhost.h"
#include "viewport.h"
#include "vjoy.h"
#include "filter.h"
#include "simplewindow.h"
#include "graphics.h"
#include "keyseq.h"

#include <shlobj_core.h>

//============================================================================================
// initialize / terminate environment
//============================================================================================
MapperEngine::MapperEngine(Callback callback, Logger logger) : 
    status(Status::init), callback(callback), logger(logger){
    event.event_as_cv = ::CreateEvent(nullptr, true, false, nullptr);
    event.idCounter = static_cast<uint64_t>(EventID::DINAMIC_EVENT);
    WinDispatcher::initSharedDispatcher();
    graphics::initialize_grahics();
}

MapperEngine::~MapperEngine(){
    // stop event-action mapping thread
    stop();
    clearScriptingEnv();
    
    // clean up graphics environment
    graphics::terminate_graphics();
}

//============================================================================================
// initialize lua scripting environment
//============================================================================================
void MapperEngine::initScriptingEnv(){
    static sol::lib libtypes[] ={
        sol::lib::base,
        sol::lib::coroutine,
        sol::lib::debug,
        sol::lib::io,
        sol::lib::math,
        sol::lib::os,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table,
        sol::lib::utf8,
    };

    scripting.lua_ptr = std::make_unique<sol::state>();
    for (auto i =0; i < sizeof(libtypes) / sizeof(libtypes[0]); i++){
        if (options.stdlib & static_cast<int32_t>(1 << i)){
            scripting.lua().open_libraries(libtypes[i]);
        }
    }
    
    //-------------------------------------------------------------------------------
    // create 'mapper' table
    //      mapper.script_path               this script file path
    //      mapper.script_dir                directory of the above file
    //      mapper.print():                  print message on console
    //      mapper.abort():                  abort mapper engine
    //      mapper.delay():                  deferred function execution
    //      mapper.register_event():         register event id
    //      mapper.unregister_event():       unregister event id
    //      mapper.get_event_name():         get name associated with event id
    //      mapper.raise_event():            raise an event
    //      mapper.set_primary_mappings():   set primary mappings
    //      mapper.add_primary_mappings();   add primary mappings
    //      mapper.set_secondary_mappings(): set secondary mappings
    //      mapper.add_secondary_mappings(); add secondary mappings
    //      mapper.device() :                open device
    //      mapper.viewport():               register viewport
    //      mapper.start_viewports():        start all viewports
    //      mapper.stop_viewports():         stop all viewports
    //      mapper.reset_viewports():        stop all viewports then remove all viewport definitions
    //      mapper.virtual_joystick():       create vJoy feeder
    //      mapper.events:                   system events table
    //-------------------------------------------------------------------------------
    auto mapper = scripting.lua().create_table();
    mapper["print"] = [this](const char* msg){
        putLog(MCONSOLE_MESSAGE, msg);
    };
    mapper["abort"] = [this](){
        putLog(MCONSOLE_ERROR, "mapper-core: abort scripting");
        abort();
    };
    mapper["delay"] = [this](const sol::object millisec_o, sol::object function_o){
        lua_c_interface(*this, "mapper:delay", [this, millisec_o, function_o](){
            auto millisec = lua_safevalue<int>(millisec_o);
            if (!millisec.has_value()){
                throw MapperException("1st parameter must be milliseconds as numeric value");
            }
            if (function_o.get_type() != sol::type::function){
                throw MapperException("2nd parameter must be a Lua function");
            }
            auto function = std::make_shared<LuaAction>(function_o);
            Event ev(static_cast<int64_t>(EventID::NILL));
            invokeActionIn(function, ev, MILLISEC(*millisec));
        });
    };
    mapper["register_event"] = [this](const sol::object obj){
        return lua_c_interface(*this, "mapper:register_event", [this, &obj](){
            auto&& desc = lua_safestring(obj);
            if (desc.length() == 0){
                throw MapperException("description of event to register must be specified as string");
            }
            return registerEvent(std::move(desc));
        });
    };
    mapper["unregister_event"] = [this](const sol::object obj){
        lua_c_interface(*this, "mapper:unregister_event", [this, &obj](){
            auto evid = lua_safevalue<int64_t>(obj);
            if (evid){
                unregisterEvent(*evid);
            }else{
                throw MapperException("event-id must be specified");
            }
        });
    };
    mapper["get_event_name"] = [this](const sol::object obj)->std::optional<std::string> {
        return lua_c_interface(*this, "mapper:get_event_name", [this, &obj]()->std::optional<std::string> {
            auto evid = lua_safevalue<int64_t>(obj);
            if (evid){
                if (event.names.count(*evid)){
                    return event.names.at(*evid);
                }else{
                    return std::nullopt;
                }
            }else{
                throw MapperException("event-id must be specified");
            }
        });
    };
    mapper["raise_event"] = [this](const sol::variadic_args va){
        lua_c_interface(*this, "mapper::raise_event", [this, &va]{
            auto&& evid = lua_safevalue<int64_t>(va[0]);
            sol::object value = va[1];
            if (!evid){
                throw MapperException("First argument must be specified and that must be event-id");
            }
            this->sendEvent(std::move(Event(*evid, std::move(value))));
        });
    };
    mapper["set_primary_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:set_primary_mappings", [this, &def](){
            setMapping("mapper.set_primary_mappings()", 0, def);
        });
    };
    mapper["add_primary_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:add_primary_mappings", [this, &def](){
            addMapping("mapper.set_primary_mappings()", 0, def);
        });
    };
    mapper["set_secondary_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:set_secondary_mappings", [this, &def](){
            setMapping("mapper.set_secondary_mappings()", 1, def);
        });
    };
    mapper["add_secondary_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:add_secondary_mappings", [this, &def](){
            addMapping("mapper.set_secondary_mappings()", 1, def);
        });
    };

    scripting.deviceManager = std::make_unique<DeviceManager>(*this);
    scripting.deviceManager->init_scripting_env(mapper);

    scripting.viewportManager = std::make_unique<ViewPortManager>(*this);
    scripting.viewportManager->init_scripting_env(mapper);

    scripting.vjoyManager = std::make_unique<vJoyManager>(*this);
    scripting.vjoyManager->init_scripting_env(scripting.lua(), mapper);

    keyseq::create_lua_env(*this, mapper);

    auto sysevents = scripting.lua().create_table();
    auto ev_change_aircraft = this->registerEvent("mapper:change_aircraft");
    sysevents["change_aircraft"] = ev_change_aircraft;
    mapper["events"] = sysevents;
    scripting.lua()["mapper"] = mapper;

    //-------------------------------------------------------------------------------
    // create filters
    //-------------------------------------------------------------------------------
    filter_create_lua_env(*this, scripting.lua());

    //-------------------------------------------------------------------------------
    // create simulator host related environments
    //-------------------------------------------------------------------------------
    scripting.simhostManager = std::make_unique<SimHostManager>(*this, ev_change_aircraft, scripting.lua());

    //-------------------------------------------------------------------------------
    // create graphics related environments
    //-------------------------------------------------------------------------------
    graphics::create_lua_env(*this, scripting.lua());

    //-------------------------------------------------------------------------------
    // set asset path & package path
    //-------------------------------------------------------------------------------
    if (options.stdlib & MOPT_STDLIB_PACKAGE){
        std::filesystem::path path(scripting.scriptPath);
        scripting.lua()["mapper"]["script_path"] = path.string();
        path.remove_filename();
        scripting.lua()["mapper"]["script_dir"] = path.string();
        scripting.lua()["mapper"]["asset_path"] = path.string();
        path /= "?.lua";
        auto package = scripting.lua()["package"];
        package["path"] = path.string();
    }

    //-------------------------------------------------------------------------------
    // set user related paths
    //-------------------------------------------------------------------------------
    wchar_t* path;
    SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &path);
    std::filesystem::path profile_path{path};
    scripting.lua()["mapper"]["profile_dir"] = profile_path.string();
    SHGetKnownFolderPath(FOLDERID_SavedGames, 0, nullptr, &path);
    std::filesystem::path saved_games_path{path};
    scripting.lua()["mapper"]["saved_games_dir"] = saved_games_path.string();
}

void MapperEngine::clearScriptingEnv(){
    // cleare all event-action maps befor delstory lua environment
    // since action may be lua function
    mapping[0] = nullptr;
    mapping[1] = nullptr;
    event.deferred_actions.clear();
    if (scripting.viewportManager){
        scripting.viewportManager->reset_viewports();
    }

    // destory lua environment
    scripting.lua_ptr = nullptr;
}

//============================================================================================
// do event loop
//============================================================================================
bool MapperEngine::run(std::string&& scriptPath){
    WinDispatcher::queue_attatcher attacher{WinDispatcher::sharedDispatcher()};
    try{
        std::unique_lock<std::mutex> lock(mutex);

        if  (status != Status::init){
            return false;
        }
        scripting.scriptPath = std::move(scriptPath);

        //-------------------------------------------------------------------------------
        // create environment for lua script
        //-------------------------------------------------------------------------------
        putLog(MCONSOLE_INFO, "mapper-core: start event-action mapping");
        initScriptingEnv();

        //-------------------------------------------------------------------------------
        // execute pre-run script
        //-------------------------------------------------------------------------------
        if (options.pre_run_script.size() > 0){
            auto result = scripting.lua().safe_script(options.pre_run_script, sol::script_pass_on_error);
            if (!result.valid()){
                sol::error err = result;
                std::ostringstream os;
                os << "mapper-core: an error occurred while evaluating pre-run script" << std::endl << err.what();
                lock.unlock();
                putLog(MCONSOLE_WARNING, os.str());
                lock.lock();
            }
        }

        //-------------------------------------------------------------------------------
        // put debug log to show information of connected monitors
        //-------------------------------------------------------------------------------
        lock.unlock();
        scripting.viewportManager->log_displays();
        lock.lock();

        //-------------------------------------------------------------------------------
        // run the script
        //-------------------------------------------------------------------------------
        status = Status::running;
        lock.unlock();
        auto result = scripting.lua().safe_script_file(scripting.scriptPath, sol::script_pass_on_error);
        if (!result.valid()){
            sol::error err = result;
            throw MapperException(err.what());
        }
        sendHostEvent(MEV_START_MAPPING, 0);
        lock.lock();

        event.view_updated_time = CLOCK::now();

        //-------------------------------------------------------------------------------
        // Event-Action mapping loop
        //-------------------------------------------------------------------------------
        while (true){
            //-------------------------------------------------------------------------------
            // collect garbage in Lua environment as needed
            //-------------------------------------------------------------------------------
            if (scripting.should_gc){
                lock.unlock();
                scripting.lua().collect_garbage();
                lock.lock();
                scripting.should_gc = false;
            }

            //-------------------------------------------------------------------------------
            // check event queue and deferred action queue then invoke action
            //-------------------------------------------------------------------------------
            now = CLOCK::now();
            bool queue_empty = true;
            if (event.queue.size() > 0){
                //-------------------------------------------------------------------------------
                // process an event
                //-------------------------------------------------------------------------------
                queue_empty = false;
                auto ev = std::move(event.queue.front());
                event.queue.pop();
                auto action = findAction(ev->getId());

                if (ev->getId() == static_cast<int64_t>(EventID::STOP)){
                    status = Status::stop;
                    lock.unlock();
                    putLog(MCONSOLE_INFO, "mapper-core: a request to stopp event-action mapping has been received");
                    lock.lock();
                    break;
                }else if (ev->getId() == static_cast<int64_t>(EventID::API_REQUEST)){
                    ApiContext* context = *ev;
                    const char* msg = nullptr;
                    try{
                        if (context->type == ApiContext::Type::start_viewports){
                            msg = "failed to enable viewports:\n";
                            scripting.viewportManager->enable_viewports();
                        }else if (context->type == ApiContext::Type::stop_viewports){
                            msg = "failed to disable viewports\n";
                            scripting.viewportManager->disable_viewports();
                        }else{
                            abort();
                        }
                        context->done = true;
                        context->result = true;
                        event.cv_for_client.notify_all();
                    }catch (MapperException& e){
                        std::ostringstream os;
                        os << "mapper-core: " << msg << e.what();
                        context->done = true;
                        context->result = false;
                        event.cv_for_client.notify_all();
                        lock.unlock();
                        putLog(MCONSOLE_WARNING, os.str());
                        lock.lock();
                    }
                }else{
                    if (logmode & MAPPER_LOG_EVENT && ev->getId() >= static_cast<int64_t>(EventID::DINAMIC_EVENT) &&
                        event.names.count(ev->getId()) > 0){
                        auto &name = event.names.at(ev->getId());
                        std::ostringstream os;
                        os << name;
                        if (ev->getType() == Event::Type::int_value){
                            os << "(" << ev->getAs<int64_t>() << ")";
                        }else if (ev->getType() == Event::Type::double_value){
                            os << "(" << ev->getAs<double>() << ")";
                        }
                        if (action){
                            os << " -> " << action->getName();
                        }
                        lock.unlock();
                        putLog(MCONSOLE_EVENT, os.str());
                        lock.lock();
                    }

                    if (action){
                        lock.unlock();
                        action->invoke(*ev, scripting.lua());
                        lock.lock();
                    }
                }
            }else if (event.deferred_actions.size() > 0 && event.deferred_actions.begin()->first < now){
                //-------------------------------------------------------------------------------
                // process a deferred action
                //-------------------------------------------------------------------------------
                queue_empty = false;
                auto& act_ev = event.deferred_actions.begin()->second;
                auto action = act_ev.get_action();
                auto& ev = act_ev.get_event();
                if (logmode & MAPPER_LOG_EVENT){
                    std::ostringstream os;
                    os << "Deferred action execution: " << action->getName();
                    lock.unlock();
                    putLog(MCONSOLE_EVENT, os.str());
                    lock.lock();
                }
                lock.unlock();
                action->invoke(ev, scripting.lua());
                lock.lock();
                event.deferred_actions.erase(event.deferred_actions.begin());
            }

            //-------------------------------------------------------------------------------
            // process touch operation on viewports
            //-------------------------------------------------------------------------------
            if (event.touch_event_occurred){
                event.touch_event_occurred = false;
                lock.unlock();
                scripting.viewportManager->process_touch_event();
                lock.lock();
            }

            //-------------------------------------------------------------------------------
            // Prioritize event-action mapping processing over view update processes or host notifications
            //-------------------------------------------------------------------------------
            if (event.queue.size() > 0 && now - event.view_updated_time < std::chrono::milliseconds(50)){
                continue;
            }
            event.view_updated_time = now;

            //-------------------------------------------------------------------------------
            // update viewport windows if needed
            //-------------------------------------------------------------------------------
            if (event.need_update_viewports){
                event.need_update_viewports = false;
                lock.unlock();
                scripting.viewportManager->update_viewports();
                lock.lock();
            }

            //-------------------------------------------------------------------------------
            // notify events to the host if needed
            //-------------------------------------------------------------------------------
            if (scripting.updated_flags){
                auto flags = scripting.updated_flags;
                scripting.updated_flags = 0;
                auto vpstat = scripting.viewportManager->get_status();
                lock.unlock();
                if (flags & UPDATED_DEVICES){
                    sendHostEvent(MEV_CHANGE_DEVICES, 0);
                }
                if (flags & UPDATED_MAPPINGS){
                    sendHostEvent(MEV_CHANGE_MAPPINGS, 0);
                }
                if (flags & UPDATED_VJOY){
                    sendHostEvent(MEV_CHANGE_VJOY, 0);
                }
                if (flags & UPDATED_VIEWPORTS){
                    sendHostEvent(MEV_CHANGE_VIEWPORTS, 0);
                }
                if (flags & UPDATED_VIEWPORTS_STATUS){
                    if (vpstat == ViewPortManager::Status::init){
                        sendHostEvent(MEV_RESET_VIEWPORTS, 0);
                    }else if (vpstat == ViewPortManager::Status::ready_to_start ||
                              vpstat == ViewPortManager::Status::suspended){
                        sendHostEvent(MEV_STOP_VIEWPORTS, 0);
                    }else if (vpstat == ViewPortManager::Status::running){
                        sendHostEvent(MEV_START_VIEWPORTS, 0);
                    }
                }
                if (flags & UPDATED_READY_TO_CAPTURE){
                    sendHostEvent(MEV_READY_TO_CAPTURE_WINDOW, 0);
                }
                if (flags & UPDATED_LOST_CAPTURED_WINDOW){
                    sendHostEvent(MEV_LOST_CAPTURED_WINDOW, 0);
                }
                lock.lock();
            }

            //-------------------------------------------------------------------------------
            // wait until event occurrence
            //-------------------------------------------------------------------------------
            if (queue_empty){
                auto deferred_num = event.deferred_actions.size();
                auto condition = [this, deferred_num]{
                    return event.queue.size() > 0 || event.deferred_actions.size() > deferred_num || 
                           status != Status::running || scripting.updated_flags || 
                           event.need_update_viewports || event.touch_event_occurred;
                };

                while (true){
                    HANDLE ev = event.event_as_cv;
                    DWORD wait_result;
                    if (deferred_num > 0){
                        auto now = CLOCK::now();
                        auto duration = event.deferred_actions.begin()->first - now;
                        auto millisec = std::chrono::duration_cast<MILLISEC>(duration).count();
                        if (millisec > 0){
                            lock.unlock();
                            wait_result = MsgWaitForMultipleObjects(1, &ev, false, millisec, QS_ALLINPUT);
                            lock.lock();
                        }else{
                            wait_result = WAIT_TIMEOUT;
                        }
                    }else{
                        lock.unlock();
                        wait_result = MsgWaitForMultipleObjects(1, &ev, false, INFINITE, QS_ALLINPUT);
                        lock.lock();
                    }
                    if (wait_result == WAIT_OBJECT_0){
                        ::ResetEvent(event.event_as_cv);
                        if (condition()){
                            break;
                        }
                    }else if (wait_result == WAIT_OBJECT_0 + 1){
                        // Window Messages has been received
                        lock.unlock();
                        WinDispatcher::sharedDispatcher().dispatch_received_messages();
                        lock.lock();
                    }else if (WAIT_TIMEOUT){
                        // It's time to process the deferred action
                        break;
                    }
                }

                if (status != Status::running){
                    break;
                }
            }
        }
        auto rc = status == Status::stop;
        event.cv_for_client.notify_all();
        lock.unlock();
        sendHostEvent(MEV_STOP_MAPPING, 0);
        clearScriptingEnv();
        return rc;
    }catch (MapperException& e){
        std::ostringstream os;
        os << "mapper-core: an error that cannot proceed event-action mapping occurred: " << std::endl << e.what();
        putLog(MCONSOLE_ERROR, os.str());
        {
            std::lock_guard lock(mutex);
            status = Status::error;
            event.cv_for_client.notify_all();
        }
        sendHostEvent(MEV_STOP_MAPPING, 0);
        clearScriptingEnv();
        return false;
    }
}

bool MapperEngine::stop(){
    sendEvent(std::move(Event(static_cast<uint64_t>(EventID::STOP))));
    return true;
}

bool MapperEngine::abort(){
    std::lock_guard lock(mutex);
    status = Status::error;
    notify_server();
    return true;
}

//============================================================================================
// event handling
//============================================================================================
uint64_t MapperEngine::registerEvent(std::string &&name){
    auto newid = event.idCounter++;
    event.names.insert(std::make_pair(newid, std::move(name)));
    return newid;
}

void MapperEngine::unregisterEvent(uint64_t evid){
    event.names.erase(evid);
}

const char* MapperEngine::getEventName(uint64_t evid) const{
    if (event.names.count(evid) > 0){
        return event.names.at(evid).c_str();
    }else{
        return nullptr;
    }
}

void MapperEngine::sendEvent(Event &&ev){
    std::lock_guard lock(mutex);
    event.queue.push(std::make_unique<Event>(std::move(ev)));
    notify_server();
}

//============================================================================================
// deferred action handling
//============================================================================================
void MapperEngine::invokeActionIn(std::shared_ptr<Action> action, const Event& ev, MILLISEC millisec){
    std::lock_guard lock(mutex);
    auto target = now + millisec;
    while (event.deferred_actions.count(target)){
        target += MILLISEC(1);
    }
    DeferredAction da(action, ev);
    event.deferred_actions.emplace(target, std::move(da));
    notify_server();
}

//============================================================================================
// finding action correspond to event
//============================================================================================
Action* MapperEngine::findAction(uint64_t evid){
    auto action = scripting.viewportManager->find_action(evid);
    if (action){
        return action;
    }else if (mapping[1].get() && mapping[1]->count(evid) > 0){
        return mapping[1]->at(evid).get();
    }else if (mapping[0].get() && mapping[0]->count(evid) > 0){
        return mapping[0]->at(evid).get();
    }
    return nullptr;
}

//============================================================================================
// funtions to expose to Lua script
//============================================================================================
void MapperEngine::setMapping(const char* function_name, int level, const sol::object& mapdef){
    mapping[level] = std::move(createEventActionMap(*this, mapdef));
    notifyUpdate(UPDATED_MAPPINGS);
}

void MapperEngine::addMapping(const char* function_name, int level, const sol::object& mapdef){
    if (mapping[level]) {
        addEventActionMap(*this, mapping[level], mapdef);
    }else{
        setMapping(function_name, level, mapdef);
    }
    notifyUpdate(UPDATED_MAPPINGS);
}

//============================================================================================
// send event to host program
//============================================================================================
void MapperEngine::sendHostEvent(MAPPER_EVENT event, int64_t data){
    if (!callback_is_inhibited){
        callback(event, data);
    }
}

//============================================================================================
// functions to expose to host program
//============================================================================================
std::vector<MapperEngine::DeviceInfo> MapperEngine::get_device_list(){
    std::lock_guard lock(mutex);
    std::vector<DeviceInfo> array;
    if (status == Status::running) {
        for (auto [key, value] : scripting.deviceManager->getDeviceInfor()) {
            array.emplace_back(key.c_str(), value.device_class);
        }
    }
	return array;
}

std::vector<CapturedWindowInfo> MapperEngine::get_captured_window_list(){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        return std::move(scripting.viewportManager->get_captured_window_list());
    }else{
        return {};
    }
}

std::vector<std::string> MapperEngine::get_captured_window_titles(uint32_t cwid){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        return std::move(scripting.viewportManager->get_captured_window_title_list(cwid));
    }else{
        return {};
    }
}

void MapperEngine::register_captured_window(uint32_t cwid, HWND hWnd){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        scripting.viewportManager->register_captured_window(cwid, hWnd);
    }
}

void MapperEngine::unregister_captured_window(uint32_t cwid){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        scripting.viewportManager->unregister_captured_window(cwid);
    }
}

std::vector<ViewportInfo> MapperEngine::get_viewport_list(){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        return std::move(scripting.viewportManager->get_viewport_list());
    }else{
        return {};
    }
}

bool MapperEngine::enable_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::running){
        ApiContext context;
        context.type = ApiContext::Type::start_viewports;
        Event ev{static_cast<uint64_t>(EventID::API_REQUEST), &context};
        lock.unlock();
        sendEvent(std::move(ev));
        lock.lock();
        event.cv_for_client.wait(lock, [this, &context](){return status != Status::running || context.done;});
        return context.result;
    }
    return true;
}

bool MapperEngine::disable_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::running){
        ApiContext context;
        context.type = ApiContext::Type::stop_viewports;
        Event ev{static_cast<uint64_t>(EventID::API_REQUEST), &context};
        lock.unlock();
        sendEvent(std::move(ev));
        lock.lock();
        event.cv_for_client.wait(lock, [this, &context](){return status != Status::running || context.done;});
        return context.result;
    }
    return true;
}

MAPPINGS_STAT MapperEngine::get_mapping_stat(){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        auto&& vstat = scripting.viewportManager->get_mappings_stat();
        auto primary = static_cast<int>(mapping[0].get() ? mapping[0]->size() : 0);
        auto secondary = static_cast<int>(mapping[1].get() ? mapping[1]->size() : 0);
        return {primary, secondary, vstat.first, vstat.second};
    }else{
        return {0, 0, 0, 0};
    }
}
