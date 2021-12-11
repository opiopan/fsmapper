//
// engine.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include "hookdll.h"
#include "engine.h"
#include "device.h"
#include "simhost.h"
#include "viewport.h"
#include "vjoy.h"
#include "filter.h"


//============================================================================================
// initialize / terminate environment
//============================================================================================
MapperEngine::MapperEngine(Callback callback, Logger logger) : 
    status(Status::init), callback(callback), logger(logger){
    event.idCounter = static_cast<uint64_t>(EventID::DINAMIC_EVENT);
}

MapperEngine::~MapperEngine(){
    // stop event-action mapping thread
    stop();

    // cleare all event-action maps befor delstory lua environment
    // since action may be lua function
    mapping[0] = nullptr;
    mapping[1] = nullptr;
    if (scripting.viewportManager){
        scripting.viewportManager->reset_viewports();
    }

    // destory lua environment
    scripting.lua_ptr = nullptr;
}

//============================================================================================
// initialize lua scripting environment
//============================================================================================
void MapperEngine::initScriptingEnvAndRun(){
    scripting.lua_ptr = std::make_unique<sol::state>();
    scripting.lua().open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);

    //-------------------------------------------------------------------------------
    // create 'mapper' table
    //      mapper.print():                  print message on console
    //      mapper.abort():                  abort mapper engine
    //      mapper.set_primery_mappings():   set primery mappings
    //      mapper.add_primery_mappings();   add primery mappings
    //      mapper.set_secondary_mappings(): set primery mappings
    //      mapper.add_secondary_mappings(); add primery mappings
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
    mapper["set_primery_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:set_primery_mappings", [this, &def](){
            setMapping("mapper.set_primery_mappings()", 0, def);
        });
    };
    mapper["add_primery_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:add_primery_mappings", [this, &def](){
            addMapping("mapper.set_primery_mappings()", 0, def);
        });
    };
    mapper["set_secondary_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:set_secondary_mappings", [this, &def](){
            setMapping("mapper.set_primery_mappings()", 1, def);
        });
    };
    mapper["add_secondary_mappings"] = [this](const sol::object def){
        lua_c_interface(*this, "mapper:add_secondary_mappings", [this, &def](){
            addMapping("mapper.set_primery_mappings()", 1, def);
        });
    };

    scripting.deviceManager = std::make_unique<DeviceManager>(*this);
    scripting.deviceManager->init_scripting_env(mapper);

    scripting.viewportManager = std::make_unique<ViewPortManager>(*this);
    scripting.viewportManager->init_scripting_env(mapper);

    scripting.vjoyManager = std::make_unique<vJoyManager>(*this);
    scripting.vjoyManager->init_scripting_env(scripting.lua(), mapper);

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
    // run the script
    //-------------------------------------------------------------------------------
    auto result = scripting.lua().safe_script_file(scripting.scriptPath, sol::script_pass_on_error);
    if (!result.valid()){
        sol::error err = result;
        throw MapperException(err.what());
    }

}

//============================================================================================
// do event loop
//============================================================================================
bool MapperEngine::run(std::string&& scriptPath){
    try{
        std::unique_lock<std::mutex> lock(mutex);

        if  (status != Status::init){
            return false;
        }
        status = Status::running;
        scripting.scriptPath = std::move(scriptPath);

        //-------------------------------------------------------------------------------
        // create environment for lua script then run
        //-------------------------------------------------------------------------------
        lock.unlock();
        initScriptingEnvAndRun();
        sendHostEvent(MEV_START_MAPPING, 0);
        lock.lock();

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
            // wait until event occurrence
            //-------------------------------------------------------------------------------
            event.cv.wait(lock, [this]{
                return this->event.queue.size() > 0 ||this->status != Status::running;
            });
            if (status != Status::running){
                break;
            }
            auto ev = std::move(event.queue.front());
            event.queue.pop();

            //-------------------------------------------------------------------------------
            // identify action correspond to event
            //-------------------------------------------------------------------------------
            auto action = findAction(ev->getId());
            if (logmode & MAPPER_LOG_EVENT && ev->getId() >= static_cast<int64_t>(EventID::DINAMIC_EVENT) &&
                event.names.count(ev->getId()) > 0){
                auto &name = event.names.at(ev->getId());
                std::ostringstream os;
                os << "Event occurred: " << name;
                if (ev->getType() == Event::Type::int_value){
                    os << "(" << ev->getAs<int64_t>() << ")";
                }
                if (action){
                    os << " -> " << action->getName();
                }
                lock.unlock();
                putLog(MCONSOLE_INFO, os.str());
                lock.lock();
            }

            //-------------------------------------------------------------------------------
            // invoke action
            //-------------------------------------------------------------------------------
            if (action){
                lock.unlock();
                action->invoke(*ev.get(), scripting.lua());
                lock.lock();
            }
        }
        auto rc = status == Status::stop;
        lock.unlock();
        sendHostEvent(MEV_STOP_MAPPING, 0);
        return rc;
    }catch (MapperException& e){
        std::ostringstream os;
        os << "mapper-core: an error that cannot proceed event-action mapping occurred: " << e.what();
        putLog(MCONSOLE_ERROR, os.str());
        {
            std::lock_guard lock(mutex);
            status = Status::error;
        }
        sendHostEvent(MEV_STOP_MAPPING, 0);
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
    event.cv.notify_all();
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
    event.cv.notify_one();
}

std::unique_ptr<Event>&& MapperEngine::receiveEvent(){
    std::unique_lock lock(mutex);
    event.cv.wait(lock, [this]{return this->event.queue.size() > 0;});
    auto ev = std::move(event.queue.front());
    event.queue.pop();
    return std::move(ev);
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
}

void MapperEngine::addMapping(const char* function_name, int level, const sol::object& mapdef){
    addEventActionMap(*this, mapping[level], mapdef);
}

//============================================================================================
// send event to host program
//============================================================================================
void MapperEngine::sendHostEvent(MAPPER_EVENT event, int64_t data){
    callback(event, data);
}

std::vector<CapturedWindowInfo> MapperEngine::get_captured_window_list(){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        return std::move(scripting.viewportManager->get_captured_window_list());
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

void MapperEngine::enable_viewports(){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        scripting.viewportManager->enable_viewports();
    }
}

void MapperEngine::disable_viewports(){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        scripting.viewportManager->disable_viewports();
    }
}
