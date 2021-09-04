//
// fs2020.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include "engine.h"
#include "fs2020.h"
#include "tools.h"
#include "action.h"

static const auto CONNECTING_INTERVAL = 500;
static const auto WATCH_DOG_ERROR_PERIOD = 10 * 1000;

enum STATIC_EVENTS{
    EVENT_SIM_START = 1,
    EVENT_1SEC,
};
static const auto DINAMIC_EVENT_MIN = 100;

enum STATIC_DATA_DEFS{
    DATA_DEF_SYSTEM = 1,
};
struct SystemData{
    char title[256];
};
static const auto DINAMIC_DATA_DEF_MIN = 100;

enum STATIC_REQESTS{
    REQUEST_SYSTEM_DATA = 1,
};
static const auto DINAMIC_REQUESt_MIN = 100;

//============================================================================================
// SimConnect event loop
//============================================================================================
FS2020::FS2020(SimHostManager& manager, int id): SimHostManager::Simulator(manager, id){
    event_simconnect = ::CreateEvent(nullptr, true, false, nullptr);
    event_interrupt = ::CreateEvent(nullptr, true, false, nullptr);
    if (event_simconnect == INVALID_HANDLE_VALUE || event_interrupt == INVALID_HANDLE_VALUE){
        throw MapperException("Failed to create event objects to communicate with FS2020");
    }
    scheduler = std::thread([this](){
        const auto ix_interrupt = 0;
        const auto ix_simconnect = 1;
        HANDLE events[] = {event_interrupt, event_simconnect};
        std::unique_lock lock(mutex);

        while (true){
            status = Status::connecting;

            //-------------------------------------------------------------------------------
            // connect to FS2020
            //-------------------------------------------------------------------------------
            while (true){
                HANDLE connection;
                auto rc = SimConnect_Open(&connection, "fsmapper", nullptr, 0, event_simconnect, 0);
                if (SUCCEEDED(rc)){
                    simconnect = connection;
                    break;
                }
                lock.unlock();
                ::WaitForSingleObject(this->event_interrupt, CONNECTING_INTERVAL);
                lock.lock();
                ::ResetEvent(event_interrupt);
                if (shouldStop){
                    return;
                }
            }
            status = Status::connected;

            //-------------------------------------------------------------------------------
            // Register data definition
            //-------------------------------------------------------------------------------
            SimConnect_AddToDataDefinition(simconnect, DATA_DEF_SYSTEM, "Title", nullptr, SIMCONNECT_DATATYPE_STRING256);
            SimConnect_RequestDataOnSimObjectType(simconnect, REQUEST_SYSTEM_DATA, DATA_DEF_SYSTEM, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);

            //-------------------------------------------------------------------------------
            // Subscribe system events
            //-------------------------------------------------------------------------------
            SimConnect_SubscribeToSystemEvent(simconnect, EVENT_SIM_START, "SimStart");
            SimConnect_SubscribeToSystemEvent(simconnect, EVENT_1SEC, "1sec");

            //-------------------------------------------------------------------------------
            // Mapping client event
            //-------------------------------------------------------------------------------
            for (auto& [name, id] : sim_events){
                SimConnect_MapClientEventToSimEvent(simconnect, id, name.c_str());
            }

            //-------------------------------------------------------------------------------
            // process SimConnect events
            //-------------------------------------------------------------------------------
            std::chrono::steady_clock::time_point watch_dog;
            std::string aircraft_name;
            while(status != Status::disconnected){
                if (shouldStop){
                    return;
                }
                lock.unlock();
                auto eventix = ::WaitForMultipleObjects(2, events, false, 2 * 1000);
                lock.lock();
                if (eventix == ix_interrupt){
                    ::ResetEvent(event_interrupt);
                }else if (eventix == ix_simconnect){
                    ::ResetEvent(event_simconnect);
                    SIMCONNECT_RECV* pData;
                    DWORD cbData;
                    lock.unlock();
                    auto rc = SimConnect_GetNextDispatch(simconnect, &pData, &cbData);
                    lock.lock();
                    if (SUCCEEDED(rc)){
                        if (pData->dwID == SIMCONNECT_RECV_ID_EVENT) {
                            SIMCONNECT_RECV_EVENT *evt = reinterpret_cast<SIMCONNECT_RECV_EVENT*>(pData);
                            if (evt->uEventID == EVENT_SIM_START){
                                SimConnect_RequestDataOnSimObjectType(simconnect, REQUEST_SYSTEM_DATA, DATA_DEF_SYSTEM, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
                            }else if (evt->uEventID == EVENT_1SEC){
                                watch_dog = std::chrono::steady_clock::now();
                                if (status == Status::connected){
                                    status = Status::start;
                                    lock.unlock();
                                    this->reportConnectivity(true, nullptr);
                                    lock.lock();
                                }
                            }
                        }else if (pData->dwID == SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE){
                            auto pObjData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*>(pData);
                            if (pObjData->dwRequestID == REQUEST_SYSTEM_DATA){
                                auto object_id = pObjData->dwObjectID;
                                auto data = reinterpret_cast<SystemData*>(&pObjData->dwData);
                                std::string new_name = data->title;
                                if (new_name != aircraft_name){
                                    aircraft_name = std::move(new_name);
                                    status = Status::start;
                                    lock.unlock();
                                    this->reportConnectivity(true, aircraft_name.c_str());
                                    lock.lock();
                                }
                            }
                        }else if (pData->dwID == SIMCONNECT_RECV_ID_QUIT){
                            status = Status::disconnected;
                        }
                    }
                }else if (status == Status::start){
                    // check watch dog
                    auto now = std::chrono::steady_clock::now();
                    if (now - watch_dog > std::chrono::milliseconds(WATCH_DOG_ERROR_PERIOD)){
                        status = Status::disconnected;
                    }
                }
            }

            isActive = false;
            lock.unlock();
            this->reportConnectivity(false, nullptr);
            ::WaitForSingleObject(event_interrupt, 5 * 1000);
            lock.lock();
            ::ResetEvent(event_interrupt);
            if (shouldStop){
                return;
            }
        }
    });
}

FS2020::~FS2020(){
    {
        std::lock_guard lock(mutex);
        shouldStop = true;
        SetEvent(event_interrupt);
    }
    scheduler.join();
}

void FS2020::changeActivity(bool isActive){
    std::lock_guard lock(mutex);
    this->isActive = isActive;
}

//============================================================================================
// Create Lua scripting environment
//============================================================================================
void FS2020::initLuaEnv(sol::state& lua){
    auto fs2020 = lua.create_table();
    fs2020["send_event"] = [this](sol::object name_o){
        auto name = std::move(lua_safestring(name_o));
        auto event_id = this->getSimEventId(name);
        this->sendSimEventId(event_id);
    };
    fs2020["event_sender"] = [this](sol::object name_o){
        auto event_name = std::move(lua_safestring(name_o));
        auto event_id = this->getSimEventId(event_name);
        std::ostringstream os;
        os << "fs2020.send_event(\"" << event_name << "\")";
        auto func_name = os.str();
        NativeAction::Function::ACTION_FUNCTION func = [event_id, this](Event&, sol::state&){
            this->sendSimEventId(event_id);
        };
        return std::make_shared<NativeAction::Function>(func_name.c_str(), func);
    };
    lua["fs2020"] = fs2020;
}

//============================================================================================
// SimConnect event handling functions
//============================================================================================
SIMCONNECT_CLIENT_EVENT_ID FS2020::getSimEventId(const std::string& event_name){
    std::lock_guard lock(mutex);
    if (sim_events.count(event_name) > 0){
        return sim_events.at(event_name);
    }else{
        auto evid = DINAMIC_EVENT_MIN + sim_events.size();
        sim_events.emplace(event_name, evid);
        if (status == Status::connected || status == Status::start){
            SimConnect_MapClientEventToSimEvent(simconnect, evid, event_name.c_str());
        }

        return evid;
    }
}

void FS2020::sendSimEventId(SIMCONNECT_CLIENT_EVENT_ID eventid){
    std::lock_guard lock(mutex);
    if (isActive && status == Status::start){
        SimConnect_TransmitClientEvent(
            simconnect, 0, eventid, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    }
}
