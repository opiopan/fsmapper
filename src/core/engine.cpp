//
// engine.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include "hookdll.h"
#include "engine.h"

//============================================================================================
// initialize / terminate environment
//============================================================================================
MapperEngine::MapperEngine(Callback callback, Logger logger) : 
    status(Status::init), callback(callback), logger(logger){
    event.idCounter = static_cast<uint64_t>(EventID::DINAMIC_EVENT);
    scripting.lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
    hookdll_startGlobalHook(nullptr, nullptr);
}

MapperEngine::~MapperEngine(){
    stop();
    hookdll_stopGlobalHook();
}

//============================================================================================
// initialize lua scripting environment
//============================================================================================
void MapperEngine::initScriptingEnvAndRun(){
    //-------------------------------------------------------------------------------
    // create 'mapper' table
    //      mapper.print():                  print message on console
    //      mapper.abort():                  abort mapper engine
    //      mapper.device() :                open device
    //      mapper.set_primery_mappings():   set primery mappings
    //      mapper.set_secondary_mappings(): set primery mappings
    //      mapper.events:                   system events table
    //-------------------------------------------------------------------------------
    auto mapper = scripting.lua.create_table();
    mapper["print"] = [this](const char* msg){
        putLog(MCONSOLE_MESSAGE, msg);
    };
    mapper["abort"] = [this](){
        putLog(MCONSOLE_ERROR, "mapper-core: abort scripting");
        abort();
    };
    scripting.deviceManager = std::make_unique<DeviceManager>(*this);
    mapper["device"] = [this](const sol::object param, sol::this_state s){
        return scripting.deviceManager->createDevice(param, s);
    };
    mapper["set_primery_mappings"] = [this](const sol::object def){
        setMapping("mapper.set_primery_mappings()", 0, def);
    };
    mapper["set_secondary_mappings"] = [this](const sol::object def){
        setMapping("mapper.set_secondary_mappings()", 1, def);
    };
    auto sysevents = scripting.lua.create_table();
    auto ev_change_aircraft = this->registerEvent("mapper:change_aircraft");
    sysevents["change_aircraft"] = ev_change_aircraft;
    mapper["events"] = sysevents;
    scripting.lua["mapper"] = mapper;

    //-------------------------------------------------------------------------------
    // create simulator host related environments
    //-------------------------------------------------------------------------------
    scripting.simhostManager = std::make_unique<SimHostManager>(*this, ev_change_aircraft, scripting.lua);

    //-------------------------------------------------------------------------------
    // test functions: will be deleted 
    //-------------------------------------------------------------------------------
    auto test = scripting.lua.create_table();
    test["messenger"] = [this](const sol::object msg_o){
        auto msg = msg_o.as<std::string>();
        std::ostringstream os;
        os << "show_msg(\"" << msg << "\")";
        auto name = os.str();
        NativeAction::Function::ACTION_FUNCTION actionfunc = [msg, this](Event &){
            std::ostringstream os;
            os << "    " << msg;
            putLog(MCONSOLE_MESSAGE, os.str().c_str());
        };
        NativeAction::Function::ACTION_FUNCTION func = [msg, this](Event &){
            std::ostringstream os;
            os << "    " << msg;
            putLog(MCONSOLE_MESSAGE, os.str().c_str());
        };
        auto action = std::make_shared<NativeAction::Function>(name.c_str(), func);
        return action;
    };
    test["capture_window"] = [this](const sol::object num_o){
        if (num_o.is<int64_t>()){
            hookdll_capture(reinterpret_cast<HWND>(num_o.as<int64_t>()));
        }
    };
    scripting.lua["test"] = test;

    //-------------------------------------------------------------------------------
    // run the script
    //-------------------------------------------------------------------------------
    auto result = scripting.lua.safe_script_file(scripting.scriptPath, sol::script_pass_on_error);
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
        lock.lock();

        while (true){
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
            if (logmode & MAPPER_LOG_EVENT && ev->getId() >= static_cast<int64_t>(EventID::DINAMIC_EVENT)){
                auto &name = event.names.at(ev->getId());
                std::ostringstream os;
                os << "Event occurred: " << name;
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
                action->invoke(*ev.get(), scripting.lua);
                lock.lock();
            }
        }
        return status == Status::stop;
    }catch (MapperException& e){
        std::ostringstream os;
        os << "mapper-core: an error that cannot proceed event-action mapping occurred: " << e.getMessage();
        putLog(MCONSOLE_ERROR, os.str());
        std::lock_guard lock(mutex);
        status = Status::error;
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
    if (mapping[0].get() && mapping[0]->count(evid) > 0){
        return mapping[0]->at(evid).get();
    }else if (mapping[1].get() && mapping[1]->count(evid) > 0){
        return mapping[1]->at(evid).get();
    }
    return nullptr;
}

//============================================================================================
// funtions to expose to Lua script
//============================================================================================
void MapperEngine::setMapping(const char* function_name, int level, const sol::object& mapdef){
    try{
        mapping[level] = std::move(createEventActionMap(*this, mapdef));
    }catch (MapperException& e){
        std::ostringstream os;
        os << function_name << ": " << e.getMessage();
        putLog(MCONSOLE_ERROR, os.str());
        abort();
    }
}

//============================================================================================
// send event to host program
//============================================================================================
void MapperEngine::sendHostEvent(MAPPER_EVENT event, int64_t data){
    callback(event, data);
}
