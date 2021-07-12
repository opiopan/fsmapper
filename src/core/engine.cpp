//
// engine.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include "engine.h"

//============================================================================================
// initialize / terminate environment
//============================================================================================
MapperEngine::MapperEngine(Callback callback, Logger logger) : 
    status(Status::init), callback(callback), logger(logger){
    event.idCounter = static_cast<uint64_t>(EventID::DINAMIC_EVENT);
    scripting.lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
}

MapperEngine::~MapperEngine(){
    stop();
}

//============================================================================================
// initialize lua scripting environment
//============================================================================================
void MapperEngine::initScriptingEnvAndRun(){
    //-------------------------------------------------------------------------------
    // create 'mapper' table
    //      mapper.print():   print message on console
    //      mapper.device() : open device
    //-------------------------------------------------------------------------------
    auto mapper = scripting.lua.create_table();
    mapper["print"] = [this](const char* msg){
        putLog(MCONSOLE_MESSAGE, msg);
    };
    scripting.deviceManager = std::make_unique<DeviceManager>(*this);
    mapper["device"] = [this](const sol::object param, sol::this_state s){
        return scripting.deviceManager->createDevice(param, s);
    };
    scripting.lua["mapper"] = mapper;

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
        {
            std::lock_guard lock(mutex);
            if  (status != Status::init){
                return false;
            }
            status = Status::running;
            scripting.scriptPath = std::move(scriptPath);
        }

        initScriptingEnvAndRun();

        std::unique_lock<std::mutex> lock(mutex);
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

            if (logmode & MAPPER_LOG_EVENT && ev->getId() >= static_cast<int64_t>(EventID::DINAMIC_EVENT)){
                auto &name = event.names.at(ev->getId());
                std::ostringstream os;
                os << "Event occurred: " << name;
                lock.unlock();
                putLog(MCONSOLE_INFO, os.str());
                lock.lock();
            }
            
        }
    }catch (MapperException& e){
        std::ostringstream os;
        os << "mapper-core: an error that cannot proceed event mapping occurred: " << e.getMessage();
        putLog(MCONSOLE_ERROR, os.str());
        std::lock_guard lock(mutex);
        status = Status::error;
    }

    return status == Status::stop;
}

bool MapperEngine::stop(){
    sendEvent(std::move(Event(static_cast<uint64_t>(EventID::STOP))));
    return true;
}

bool MapperEngine::abort(){
    std::lock_guard lock(mutex);
    status = Status::error;
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
