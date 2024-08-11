//
// fs2020.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <format>
#include <unordered_map>
#include "engine.h"
#include "fs2020.h"
#include "tools.h"
#include "action.h"
#include "mobiflight_wasm.h"

static constexpr auto CONNECTING_INTERVAL = 500;
static constexpr auto WATCH_DOG_ERROR_PERIOD = 30 * 1000;

enum STATIC_EVENTS{
    EVENT_SIM_START = 1,
    EVENT_1SEC,
};
static const auto DYNAMIC_EVENT_MIN = 100;

enum STATIC_DATA_DEFS{
    DATA_DEF_SYSTEM = 1,
};
struct SystemData{
    char title[256];
};
static constexpr auto DYNAMIC_DATA_DEF_MIN = 100;

enum STATIC_REQESTS{
    REQUEST_SYSTEM_DATA = 1,
};
static constexpr auto DYNAMIC_REQUEST_MIN = 100;

//============================================================================================
// represatation of observed simulation variables
//============================================================================================
struct SimVar{
    std::string name;
    std::string unit;
    uint64_t eventid;
    double epsilon;
    double value = 0.;

    SimVar() = delete;
    SimVar(const char* name, const char* unit, uint64_t eventid, double epsilon = 0.): 
        name(name), unit(unit), eventid(eventid), epsilon(epsilon){}
    SimVar(SimVar&& src){*this = std::move(src);}
    SimVar& operator = (SimVar&& src){
        name = std::move(src.name);
        unit = std::move(src.unit);
        epsilon = src.epsilon;
        value = src.value;
        return *this;
    }
};

static constexpr auto SIMVAR_GROUP_DEFINITION_ID_OFFSET = 1000;

struct FS2020::SimVarGroup{
    bool is_registerd = false;
    std::vector<SimVar> simvars;

    SimVarGroup() = default;
    SimVarGroup(SimVarGroup&& src){*this = std::move(src);}
    ~SimVarGroup() = default;
    SimVarGroup& operator = (SimVarGroup&& src){
        is_registerd = src.is_registerd;
        simvars = std::move(src.simvars);
        return *this;
    }

    void subscribe(HANDLE simconnect, DWORD id){
        if (!is_registerd){
            for (auto& simvar : simvars){
                auto hr = ::SimConnect_AddToDataDefinition(
                    simconnect,
                    SIMVAR_GROUP_DEFINITION_ID_OFFSET + id, 
                    simvar.name.c_str(),
                    simvar.unit.c_str(),
                    SIMCONNECT_DATATYPE_FLOAT64,
                    simvar.epsilon);
            }
            auto hr = ::SimConnect_RequestDataOnSimObjectType(
                simconnect,
                SIMVAR_GROUP_DEFINITION_ID_OFFSET + id,
                SIMVAR_GROUP_DEFINITION_ID_OFFSET + id,
                0, SIMCONNECT_SIMOBJECT_TYPE_USER
            );
            is_registerd = true;
        }
    }
    void unsubscribe(HANDLE simconnect, DWORD id){
        if (is_registerd){
            ::SimConnect_ClearDataDefinition(simconnect, SIMVAR_GROUP_DEFINITION_ID_OFFSET + id);
        }
    }
};

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
                auto rc = SimConnect_Open(&connection, "fsmapper", nullptr, 0, 0, 0);
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
            // Register data definition then subscribe taht
            //-------------------------------------------------------------------------------
            ::SimConnect_AddToDataDefinition(simconnect, DATA_DEF_SYSTEM, "Title", nullptr, SIMCONNECT_DATATYPE_STRING256);
            ::SimConnect_RequestDataOnSimObjectType(simconnect, REQUEST_SYSTEM_DATA, DATA_DEF_SYSTEM, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
            //-------------------------------------------------------------------------------
            // Subscribe system events
            //-------------------------------------------------------------------------------
            ::SimConnect_SubscribeToSystemEvent(simconnect, EVENT_SIM_START, "SimStart");
            ::SimConnect_SubscribeToSystemEvent(simconnect, EVENT_1SEC, "1sec");

            //-------------------------------------------------------------------------------
            // Mapping client event
            //-------------------------------------------------------------------------------
            for (auto& [name, id] : sim_events){
                ::SimConnect_MapClientEventToSimEvent(simconnect, id, name.c_str());
            }

            //-------------------------------------------------------------------------------
            // process SimConnect events
            //-------------------------------------------------------------------------------
            while(status != Status::disconnected){
                if (shouldStop){
                    return;
                }else if (needToUpdateMfwasm){
                    needToUpdateMfwasm = false;
                    lock.unlock();
                    mfwasm_update_simvar_observation();
                    lock.lock();
                }
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                ::SimConnect_CallDispatch(simconnect, [](SIMCONNECT_RECV* pData, DWORD cbData, void *context)->void{
                    auto self = reinterpret_cast<FS2020*>(context);
                    self->processSimConnectReceivedData(pData, cbData);
                }, this);
                lock.lock();
                if (status == Status::start){
                    // check watch dog
                    auto now = std::chrono::steady_clock::now();
                    if (now - watch_dog > std::chrono::milliseconds(WATCH_DOG_ERROR_PERIOD)){
                        status = Status::disconnected;
                    }
                }
            }

            //-------------------------------------------------------------------------------
            // clean up SimConnect connection
            //-------------------------------------------------------------------------------
            isActive = false;
            aircraftName = "";
            ::SimConnect_ClearClientDataDefinition(simconnect, DATA_DEF_SYSTEM);
            lock.unlock();
            mfwasm_stop();
            lock.lock();
            simconnect = nullptr;
            representativeWindow = 0;
            lock.unlock();
            this->reportConnectivity(false, MAPPER_SIM_NONE, nullptr, nullptr);
            ::WaitForSingleObject(event_interrupt, 5 * 1000);
            lock.lock();
            ::ResetEvent(event_interrupt);
            if (shouldStop){
                return;
            }
        }
    });
}

void FS2020::processSimConnectReceivedData(SIMCONNECT_RECV* pData, DWORD cbData){
    std::unique_lock lock(mutex);
    if (pData->dwID == SIMCONNECT_RECV_ID_OPEN){
        auto data = reinterpret_cast<SIMCONNECT_RECV_OPEN*>(pData);
        auto&& message = std::format(
            "msfs: Connection via SimConnect has been established:\n"
            "    Product Name       : {}\n"
            "    Product Version    : {}.{}\n"
            "    Product Build:     : {}.{}\n"
            "    SimConnect Version : {}.{}\n"
            "    SimConnect Build   : {}.{}",
            data->szApplicationName,
            data->dwApplicationVersionMajor, data->dwApplicationVersionMinor,
            data->dwApplicationBuildMajor, data->dwApplicationBuildMinor,
            data->dwSimConnectVersionMajor, data->dwSimConnectVersionMinor,
            data->dwSimConnectBuildMajor, data->dwSimConnectBuildMinor);
        mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, message);
        if (data->dwApplicationBuildMajor == 10){
            simid = MAPPER_SIM_FSX;
            simname = "fsx";
        }else if (data->dwApplicationVersionMajor ==11){
            simid = MAPPER_SIM_FS2020;
            simname = "fs2020";
        }else if (data->dwApplicationVersionMajor == 12){
            simid = MAPPER_SIM_FS2024;
            simname = "fs2024";
        }else{
            simid = MAPPER_SIM_SIMCONNECT;
            simname = "simconnect";
        }
    }else if (pData->dwID == SIMCONNECT_RECV_ID_EVENT) {
        auto evt = reinterpret_cast<SIMCONNECT_RECV_EVENT*>(pData);
        if (evt->uEventID == EVENT_SIM_START){
            ::SimConnect_RequestDataOnSimObjectType(simconnect, REQUEST_SYSTEM_DATA, DATA_DEF_SYSTEM, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
        }else if (evt->uEventID == EVENT_1SEC){
            watch_dog = std::chrono::steady_clock::now();
            if (status == Status::connected){
                mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "msfs: Detected that the simulator is started");
                status = Status::start;
                lock.unlock();
                this->reportConnectivity(true, simid, simname, nullptr);
                lock.lock();
            }
            if (is_waiting_enum_input_event){
                enum_input_event_id++;
                ::SimConnect_EnumerateInputEvents(simconnect, enum_input_event_id);
                mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "fs2020: Requested to enumerate InputEvents");
            }
        }
    }else if (pData->dwID == SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE){
        auto pObjData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*>(pData);
        if (pObjData->dwRequestID == REQUEST_SYSTEM_DATA){
            auto object_id = pObjData->dwObjectID;
            auto data = reinterpret_cast<SystemData*>(&pObjData->dwData);
            std::string new_name = data->title;
            if (new_name != aircraftName){
                aircraftName = std::move(new_name);
                status = Status::start;
                watch_dog = std::chrono::steady_clock::now();
                updateRepresentativeWindow();
                lock.unlock();
                mfwasm_start(*this, simconnect);
                this->reportConnectivity(true, simid, simname, aircraftName.c_str());
                lock.lock();
                for (auto i = 0; i < simvar_groups.size(); i++){
                    subscribeSimVarGroup(i);
                }
                enum_input_event_id++;
                input_events.clear();
                is_waiting_enum_input_event = true;
            }
        }else if (pObjData->dwRequestID >= SIMVAR_GROUP_DEFINITION_ID_OFFSET && 
            pObjData->dwRequestID < SIMVAR_GROUP_DEFINITION_ID_OFFSET + simvar_groups.size()){
            auto& group = simvar_groups[pObjData->dwRequestID - SIMVAR_GROUP_DEFINITION_ID_OFFSET];
            auto data = reinterpret_cast<double*>(&pObjData->dwData);
            for (auto i = 0; i < group.simvars.size(); i++){
                auto& simvar = group.simvars[i];
                auto delta = simvar.value - data[i];
                if (delta > simvar.epsilon || delta < -simvar.epsilon){
                    simvar.value = data[i];
                    Event event(simvar.eventid, data[i]);
                    mapper_EngineInstance()->sendEvent(std::move(event));
                }
            }
        }
    }else if (pData->dwID == SIMCONNECT_RECV_ID_CLIENT_DATA){
        auto pClientData = reinterpret_cast<SIMCONNECT_RECV_CLIENT_DATA*>(pData);
        lock.unlock();
        mfwasm_process_client_data(pClientData);
        lock.lock();
    }else if (pData->dwID == SIMCONNECT_RECV_ID_ENUMERATE_INPUT_EVENTS){
        auto pObjData = reinterpret_cast<SIMCONNECT_RECV_ENUMERATE_INPUT_EVENTS*>(pData);
        if (pObjData->dwRequestID == enum_input_event_id){
            std::ostringstream os;
            os << "fs2020: Received an data to enumerate InputEvent: num = " << pObjData->dwArraySize;
            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, os.str());
            is_waiting_enum_input_event = false;
            for (DWORD i = 0; i < pObjData->dwArraySize; i++){
                auto data = std::make_unique<SIMCONNECT_INPUT_EVENT_DESCRIPTOR>(pObjData->rgData[i]);
                input_events[data->Name] = std::move(data);
            }
        }
    }else if (pData->dwID == SIMCONNECT_RECV_ID_EXCEPTION){
        auto pObjData = reinterpret_cast<SIMCONNECT_RECV_EXCEPTION*>(pData);
        std::ostringstream os;
        os << "fs2020: Received an exception data:\n";
        os << "    dwException: " << pObjData->dwException;
        os << "\n    dwSendID: " << pObjData->dwSendID;
        os << "\n    dwIndex: " << pObjData->dwIndex;
        mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, os.str());
    }else if (pData->dwID == SIMCONNECT_RECV_ID_QUIT){
        status = Status::disconnected;
    }
}


FS2020::~FS2020(){
    {
        std::lock_guard lock(mutex);
        shouldStop = true;
        SetEvent(event_interrupt);
    }
    scheduler.join();
    mfwasm_stop();
}

void FS2020::changeActivity(bool isActive){
    std::lock_guard lock(mutex);
    this->isActive = isActive;
}

void FS2020::updateMfwasm(){
    std::lock_guard lock(mutex);
    needToUpdateMfwasm = true;
    SetEvent(event_interrupt);
}

void FS2020::updateRepresentativeWindow(){
    representativeWindow = 0;
    ::EnumWindows([](HWND hwnd, LPARAM lparam)->BOOL{
        static std::string class_name{"AceApp"};
        static const char title[] = "Microsoft Flight Simulator";
        char buf[256];
        auto self = reinterpret_cast<FS2020*>(lparam);
        if (::GetClassNameA(hwnd, buf, sizeof(buf)) > 0){
            if (class_name == buf){
                if (::GetWindowTextA(hwnd, buf, sizeof(buf)) > 0){
                    if (strncmp(title, buf, sizeof(title) - 1) == 0){
                        self->representativeWindow = hwnd;
                        return false;
                    }
                }
            }
        }
        return true;
    }, reinterpret_cast<LPARAM>(this));
}

//============================================================================================
// Create Lua scripting environment
//============================================================================================
void FS2020::initLuaEnv(sol::state& lua){
    auto fs2020 = lua.create_table();

    using eparam = std::optional<int64_t>;
    constexpr auto evalue = [](const eparam& param){return param ? *param : 0;};
    fs2020["send_event"] = [this](sol::object name_o, eparam param1, eparam param2, eparam param3, eparam param4, eparam param5){
        auto name = std::move(lua_safestring(name_o));
        auto event_id = this->getSimEventId(name);
        this->sendSimEventId(event_id, evalue(param1), evalue(param2), evalue(param3), evalue(param4), evalue(param5));
    };

    fs2020["event_sender"] = [this, evalue](
        sol::object name_o, eparam param1, eparam param2, eparam param3, eparam param4, eparam param5){
        auto event_name = std::move(lua_safestring(name_o));
        auto event_id = this->getSimEventId(event_name);
        auto p1 = evalue(param1);
        auto p2 = evalue(param2);
        auto p3 = evalue(param3);
        auto p4 = evalue(param4);
        auto p5 = evalue(param5);
        std::ostringstream os;
        os << "fs2020.send_event(\"" << event_name << "\")";
        auto func_name = os.str();
        NativeAction::Function::ACTION_FUNCTION func = [event_id, p1, p2, p3, p4, p5, this](Event&, sol::state&){
            this->sendSimEventId(event_id, p1, p2, p3, p4, p5);
        };
        return std::make_shared<NativeAction::Function>(func_name.c_str(), func);
    };

    fs2020["add_observed_simvars"] = [this](sol::object obj){
        lua_c_interface(*mapper_EngineInstance(), "fs2020.add_observed_simvars", [this, &obj]{
            addObservedSimVars(obj);
        });
    };

    fs2020["clear_observed_simvars"] = [this]{
        lua_c_interface(*mapper_EngineInstance(), "fs2020.clear_observed_simvars", [this]{
            clearObservedSimVars();
        });
    };

    fs2020["execute_input_event"] = [this](const std::string& event_name, sol::object event_value){
        lua_c_interface(*mapper_EngineInstance(), "fs2020.execute_input_event", [this, &event_name, event_value]{
            if (event_value.get_type() == sol::type::number){
                raiseInputEvent(event_name, event_value.as<double>());
            }else if (event_value.get_type() == sol::type::string){
                raiseInputEvent(event_name, event_value.as<std::string>());
            }else{
                throw MapperException("the data type of event value specified as the 2nd argument must be a number or string");
            }
        });
    };

    fs2020["input_event_executer"] = [this](const std::string &event_name, sol::object event_value) {
        return lua_c_interface(*mapper_EngineInstance(), "fs2020.input_event_executer", [this, &event_name, event_value]{
            std::ostringstream os;
            os << "fs2020.execute_input_event(\"" << event_name << "\")";
            auto func_name = os.str();
            if (event_value.get_type() == sol::type::number){
                auto value = event_value.as<double>();
                NativeAction::Function::ACTION_FUNCTION func = [this, func_name, event_name, value](Event&, sol::state&){
                    raiseInputEvent(event_name, value);
                };
                return std::make_shared<NativeAction::Function>(func_name.c_str(), func);
            }else if (event_value.get_type() == sol::type::string){
                auto&& value = event_value.as<std::string>();
                NativeAction::Function::ACTION_FUNCTION func = [this, func_name, event_name, value](Event&, sol::state&){
                    raiseInputEvent(event_name, value);
                };
                return std::make_shared<NativeAction::Function>(func_name.c_str(), func);
            }else if (event_value.get_type() == sol::type::nil){
                NativeAction::Function::ACTION_FUNCTION func = [this, func_name, event_name](Event& event, sol::state&){
                    auto type = event.getType();
                    if (type == Event::Type::int_value || type == Event::Type::double_value){
                        raiseInputEvent(event_name, static_cast<double>(event));
                    }else if (type ==Event::Type::string_value){
                        raiseInputEvent(event_name, static_cast<const char*>(event));
                    }else{
                        std::ostringstream os;
                        os << "fs2020.input_event_executer(): The InputEvent could not be executed due to incorreect value type.";
                        mapper_EngineInstance()->putLog(MCONSOLE_WARNING, os.str());
                    }
                };
                return std::make_shared<NativeAction::Function>(func_name.c_str(), func);
            }else{
                throw MapperException("the data type of event value specified as the 2nd argument must be a number or string");
            }
        });
    };


    mfwasm_create_lua_env(*this, fs2020);
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
        auto evid = DYNAMIC_EVENT_MIN + sim_events.size();
        sim_events.emplace(event_name, evid);
        if (status == Status::connected || status == Status::start){
            SimConnect_MapClientEventToSimEvent(simconnect, evid, event_name.c_str());
        }

        return evid;
    }
}

void FS2020::sendSimEventId(SIMCONNECT_CLIENT_EVENT_ID eventid, DWORD param1, DWORD param2, DWORD param3, DWORD param4, DWORD param5){
    std::lock_guard lock(mutex);
    if (isActive && status == Status::start){
        SimConnect_TransmitClientEvent_EX1(
            simconnect, 0, eventid, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY,
            param1, param2, param3, param4, param5);
    }
}

//============================================================================================
// SimVar handling functions
//============================================================================================
void FS2020::addObservedSimVars(sol::object obj){
    if (obj.get_type() == sol::type::table){
        sol::table def = obj;
        SimVarGroup group;
        for (auto i = 1; i <= def.size(); i++){
            auto item = def[i];
            if (item.get_type() == sol::type::table){
                sol::table simvar = item;
                auto name = lua_safestring(simvar["name"]);
                auto unit = lua_safestring(simvar["unit"]);
                auto eventid = lua_safevalue<uint64_t>(simvar["event"]);
                auto epsilon = lua_safevalue<double>(simvar["epsilon"]);
                if (name.size() == 0 || !eventid){
                    throw MapperException("invarid arguments, SimVar defenition to observe must contain "
                                          "\"name\" parameter and \"event\" parameter at least");
                }
                group.simvars.emplace_back(name.c_str(), unit.c_str(), *eventid, epsilon ? *epsilon : 0.);
            }else{
                throw MapperException("argument must be a array of tables");
            }
        }

        std::lock_guard lock(mutex);
        simvar_groups.emplace_back(std::move(group));
        if (status == Status::start){
            subscribeSimVarGroup(simvar_groups.size() - 1);
        }
    }else{
        throw MapperException("argument must be a array of tables");
    }
}

void FS2020::subscribeSimVarGroup(size_t ix){
    simvar_groups[ix].subscribe(simconnect, ix);
}

void FS2020::clearObservedSimVars(){
    std::lock_guard lock(mutex);
    if (status == Status::start){
        unsubscribeSimVarGroups();
    }
    simvar_groups.clear();
}

void FS2020::unsubscribeSimVarGroups(){
    for (auto i = 0; i < simvar_groups.size(); i++){
        simvar_groups[i].unsubscribe(simconnect, i);
    }
}

//============================================================================================
// InputEvent handling functions
//============================================================================================
void FS2020::raiseInputEvent(const std::string& event_name, double value){
    if (input_events.count(event_name) > 0){
        auto& event_def = input_events[event_name];
        if (event_def->eType == SIMCONNECT_INPUT_EVENT_TYPE_DOUBLE){
            ::SimConnect_SetInputEvent(simconnect, event_def->Hash, sizeof(value), &value);
        }else{
            throw MapperException("the data type for the specified InputEvent is string, but the specified data type is number");
        }
    }else{
        std::ostringstream os;
        os << "fs2020.execute_input_event(): The specified InputEvent name, " << event_name << ", does not exist.";
        mapper_EngineInstance()->putLog(MCONSOLE_WARNING, os.str());
    }
}

void FS2020::raiseInputEvent(const std::string& event_name, const std::string& value){
    if (input_events.count(event_name) > 0){
        auto& event_def = input_events[event_name];
        if (event_def->eType == SIMCONNECT_INPUT_EVENT_TYPE_STRING){
            ::SimConnect_SetInputEvent(simconnect, event_def->Hash, value.length() + 1, const_cast<char*>(value.c_str()));
        }else{
            throw MapperException("the data type for the specified InputEvent is number, but the specified data type is string");
        }
    }else{
        std::ostringstream os;
        os << "fs2020.execute_input_event(): The specified InputEvent name, " << event_name << ", does not exist.";
        mapper_EngineInstance()->putLog(MCONSOLE_WARNING, os.str());
    }
}
