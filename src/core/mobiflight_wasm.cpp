//
// mobiflight_wasm.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "mobiflight_wasm.h"

#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <string.h>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>

#include "mappercore_inner.h"
#include "tools.h"
#include "engine.h"
#include "fs2020.h"

using namespace nlohmann;

struct simvar;
class mobiflight_wasm;
std::mutex mutex;
std::vector<simvar> simvars;
std::vector<simvar> holding_simvars;
bool need_to_clear = true;
uint32_t registered_vars = 0;
std::unique_ptr<mobiflight_wasm> connection;

static void show_debug_message(const char* funcname, HRESULT result){
    std::ostringstream os;
    os << "mfwasm: " << funcname << " has been failed [" << result << "]";
    mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, os.str());
}

static constexpr inline void debug_assert(const char* funcname, HRESULT result){
    if (result != S_OK){
        show_debug_message(funcname, result);
    }
}

//============================================================================================
// Observed lvalue
//============================================================================================
static constexpr auto varid_offset = 1000;

struct simvar{
    std::string var_def;
    uint64_t eventid;
    float value = 0.f;
    float epsilon = 0.f;

    simvar() = delete;
    simvar(const std::string& var_def, uint64_t eventid) : var_def(var_def), eventid(eventid){}
    simvar(const simvar&) = delete;
    simvar(simvar&& src){*this = std::move(src);}
    ~simvar() = default;
    simvar& operator = (const simvar&) = delete;
    simvar& operator = (simvar&& src){
        var_def = std::move(src.var_def);
        eventid = src.eventid;
        value = src.value;
        epsilon = src.epsilon;
        return *this;
    }
};

//============================================================================================
// Represatation of channel comunicating with MobiFlight WASM module
//============================================================================================
static const char* CHANNEL_SUFFIX_COMMAND = ".Command";
static const char* CHANNEL_SUFFIX_RESPONSE = ".Response";
static const char* CHANNEL_SUFFIX_SIMVAR = ".LVars";
static constexpr auto simvar_data_size = 4096;
static constexpr auto mobiflight_message_size = 1024;

struct channel{
    struct client_data{
        std::string name;
        SIMCONNECT_CLIENT_DATA_ID id;
    };
    HANDLE simconnect = nullptr;
    std::string name;
    client_data command;
    client_data response;
    client_data simvar;
    SIMCONNECT_CLIENT_DATA_DEFINITION_ID msgdata_defid;
    char* command_buf;

    channel(const char* name, SIMCONNECT_CLIENT_DATA_ID id_from, SIMCONNECT_CLIENT_DATA_DEFINITION_ID msgdata_defid) :
        name(name), command{name, id_from}, response{name, id_from + 1}, simvar{name, id_from * 2}, msgdata_defid(msgdata_defid){
        command.name += CHANNEL_SUFFIX_COMMAND;
        response.name += CHANNEL_SUFFIX_RESPONSE;
        simvar.name += CHANNEL_SUFFIX_SIMVAR;
        command_buf = new char[mobiflight_message_size];
    }
    ~channel(){
        delete command_buf;
    }

    void initialize(HANDLE simconnect, bool ommit_simvar = false){
        this->simconnect = simconnect;
        debug_assert(
            "SimConnect_MapClientDataNameToID()", 
            ::SimConnect_MapClientDataNameToID(simconnect, command.name.c_str(), command.id));
        debug_assert(
            "SimConnect_CreateClientData",
            ::SimConnect_CreateClientData(simconnect, command.id, mobiflight_message_size, 0));
        debug_assert(
            "SimConnect_MapClientDataNameToID()", 
            ::SimConnect_MapClientDataNameToID(simconnect, response.name.c_str(), response.id));
        debug_assert(
            "SimConnect_CreateClientData",
            ::SimConnect_CreateClientData(simconnect, response.id, mobiflight_message_size, 0));

        debug_assert(
            "SimConnect_AddToClientDataDefinition",
            ::SimConnect_AddToClientDataDefinition(simconnect, msgdata_defid, 0, mobiflight_message_size));
        debug_assert(
            "SimConnect_RequestClientData",
            SimConnect_RequestClientData(simconnect, response.id, msgdata_defid, msgdata_defid,
                                         SIMCONNECT_CLIENT_DATA_PERIOD::SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, 
                                         SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED));

        if (!ommit_simvar){
            debug_assert(
                "SimConnect_MapClientDataNameToID()", 
                ::SimConnect_MapClientDataNameToID(simconnect, simvar.name.c_str(), simvar.id));
            debug_assert(
                "SimConnect_CreateClientData",
                ::SimConnect_CreateClientData(simconnect, simvar.id, simvar_data_size, 0));
        }
    }

    void send_command(const char* cmd){
        command_buf[0] = 0;
        debug_assert(
            "SimConnect_SetClientData",
            SimConnect_SetClientData(simconnect, command.id, msgdata_defid, 0, 0, mobiflight_message_size, command_buf));
        strcpy_s(command_buf, mobiflight_message_size, cmd);
        debug_assert(
            "SimConnect_SetClientData",
            SimConnect_SetClientData(simconnect, command.id, msgdata_defid, 0, 0, mobiflight_message_size, command_buf));
    }
};

//============================================================================================
// Communication with MobiFlight WASM module
//============================================================================================
class mobiflight_wasm{
    FS2020& fs2020;
    enum class connection_status{init, connected};
    connection_status status = connection_status::init;
    HANDLE simconnect;
    channel initial_channel{"MobiFlight", 0, 0};
    channel my_channel;

public:
    mobiflight_wasm() = delete;

    mobiflight_wasm(FS2020& fs2020, HANDLE simconnect, const char* client_name) : fs2020(fs2020), simconnect(simconnect), my_channel{client_name, 3, 1} {
        debug_assert(
            "SimConnect_MapClientDataNameToID", 
            ::SimConnect_MapClientDataNameToID(simconnect, initial_channel.command.name.c_str(), initial_channel.command.id));
        initial_channel.initialize(simconnect, true);
        my_channel.initialize(simconnect);

        // Sometimes first command after reconnect is ignored. Therefore send just some arbitrary command.
        initial_channel.send_command("");
        std::string cmd{"MF.Clients.Add."};
        cmd += my_channel.name;
        initial_channel.send_command(cmd.c_str());
    }

    ~mobiflight_wasm(){
    }

    void process_wasm_response(SIMCONNECT_RECV_CLIENT_DATA* data){
        if (data->dwDefineID == initial_channel.msgdata_defid){
            // response of command via initial channel
            if (status == connection_status::init){
                auto data_buf = reinterpret_cast<const char*>(&data->dwData);
                auto object = json::parse(data_buf);
                static const char* key = "Name";
                if (object.count(key)){
                    auto value = object[key];
                    if (value.is_string()){
                        auto&& name = value.get<std::string>();
                        if (name == my_channel.name){
                            status = connection_status::connected;
                            update_simvar_observation();
                        }
                    }
                }
            }
        }else if (data->dwDefineID == my_channel.msgdata_defid){
            // response of command via fsmapper dedicated channel
            auto data_buf = reinterpret_cast<const char*>(&data->dwData);
            auto c = data_buf[0];
        }else if (data->dwDefineID >= varid_offset && data->dwDefineID < varid_offset + simvars.size()){
            auto& simvar = simvars[data->dwDefineID - varid_offset];
            auto value = *reinterpret_cast<const float*>(&data->dwData);
            auto delta = value - simvar.value;
            if (delta > simvar.epsilon || delta < -simvar.epsilon){
                simvar.value = value;
                Event event(simvar.eventid, static_cast<double>(value));
                mapper_EngineInstance()->sendEvent(std::move(event));
            }
        }
    }

    void update_simvar_observation(){
        if (status != connection_status::connected){
            return;
        }
        if (need_to_clear){
            my_channel.send_command("MF.SimVars.Clear");
            for (auto i = 0; i << simvars.size(); i++){
                debug_assert("SimConnect_ClearClientDataDefinition", ::SimConnect_ClearClientDataDefinition(simconnect, varid_offset + i));
            }
            simvars.clear();
            need_to_clear = false;
        }
        for (auto& var : holding_simvars){
            auto i = simvars.size();
            auto varid = i + varid_offset;
            debug_assert(
                "SimConnect_AddToClientDataDefinition",
                ::SimConnect_AddToClientDataDefinition(simconnect, varid, sizeof(float) * i, sizeof(float), var.epsilon));
            debug_assert(
                "SimConnect_RequestClientData", 
                ::SimConnect_RequestClientData(
                    simconnect, my_channel.simvar.id, varid, varid,
                    SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET,
                    SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED));
            std::string cmd{"MF.SimVars.Add."};
            cmd += var.var_def;
            my_channel.send_command(cmd.c_str());
            simvars.emplace_back(std::move(var));
        }
        holding_simvars.clear();
    }

    void notify_need_to_update(){
        fs2020.updateMfwasm();
    }

    void execute_rpn(const char* rpn){
        std::string cmd{"MF.SimVars.Set."};
        cmd += rpn;
        my_channel.send_command(cmd.c_str());
    }
};

//============================================================================================
// functions for SimConnect thread
//============================================================================================
void mfwasm_start(FS2020& fs2020, HANDLE handle){
    std::lock_guard lock(mutex);
    if (!connection){
        std::ostringstream os;
        os << "fsmapper:" << ::GetCurrentProcessId();
        connection = std::move(std::make_unique<mobiflight_wasm>(fs2020, handle, os.str().c_str()));
    }
}

void mfwasm_stop(){
    std::lock_guard lock(mutex);
    connection = nullptr;
    registered_vars = 0;
    need_to_clear = true;
}

void mfwasm_process_client_data(SIMCONNECT_RECV_CLIENT_DATA* data){
    std::lock_guard lock(mutex);
    if (connection){
        connection->process_wasm_response(data);
    }
}

void mfwasm_update_simvar_observation(){
    std::lock_guard lock(mutex);
    if (connection){
        connection->update_simvar_observation();
    }
}

//============================================================================================
// Interface for Lua script
//============================================================================================
static void add_observed_simvars(sol::object& obj){
    std::lock_guard lock(mutex);
    if (obj.get_type() == sol::type::table){
        sol::table defs = obj;
        for (auto i = 1; i <= defs.size(); i++){
            auto item = defs[i];
            if (item.get_type() == sol::type::table){
                sol::table def = item;
                auto&& var = lua_safestring(item["rpn"]);
                auto event = lua_safevalue<uint64_t>(item["event"]);
                auto initial_value = lua_safevalue<float>(item["initial_value"]);
                auto epsilon = lua_safevalue<float>(item["epsilon"]);
                if (var.length() == 0 || !event){
                    throw MapperException("invarid simvar definition, \"var\" parameter and \"event\" parameter must be specified");
                }
                holding_simvars.emplace_back(var, *event);
                if (initial_value){
                    holding_simvars[holding_simvars.size() - 1].value = *initial_value;
                }
                if (epsilon){
                    holding_simvars[holding_simvars.size() - 1].epsilon = *epsilon;
                }
            }
        }
        if (connection){
            connection->notify_need_to_update();
        }
    }
}

static void clear_observed_simvars(){
    std::lock_guard lock(mutex);
    need_to_clear = true;
    holding_simvars.clear();
    if (connection){
        connection->notify_need_to_update();
    }
}

static void execute_rpn(FS2020& fs2020, const char* rpn){
    fs2020.run_with_lock([rpn]{
        std::lock_guard lock(mutex);
        if (connection){
            connection->execute_rpn(rpn);
        }else{
            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "mfwasm: [execute_rpn()] FS2020 is not running");
        }
    });
}

void mfwasm_create_lua_env(FS2020& fs2020, sol::table& fs2020_table){
    auto table = mapper_EngineInstance()->getLuaState().create_table();

    table["add_observed_data"] = [](sol::object obj){
        lua_c_interface(*mapper_EngineInstance(), "fs2020.mfwasm.add_observed_data", [&obj]{
            add_observed_simvars(obj);
        });
    };
    table["clear_observed_data"] = []{
        lua_c_interface(*mapper_EngineInstance(), "fs2020.mfwasm.clear_observed_data", []{
            clear_observed_simvars();
        });
    };
    table["execute_rpn"] = [&fs2020](sol::object obj) {
        lua_c_interface(*mapper_EngineInstance(), "fs2020.mfwasm.execute_rpn", [&fs2020, &obj] {
            auto rpn = lua_safestring(obj);
            if (rpn.size() == 0){
                throw MapperException("invalid argument");
            }
            execute_rpn(fs2020, rpn.c_str());
        });
    };
    table["rpn_executer"] = [&fs2020](sol::object obj) {
        return lua_c_interface(*mapper_EngineInstance(), "fs2020.mfwasm.rpn_executer", [&fs2020, &obj] {
            auto rpn = lua_safestring(obj);
            if (rpn.size() == 0) {
                throw MapperException("invalid argument");
            }
            std::ostringstream os;
            os << "fs2020.mfwasm.execute_rpn(\"" << rpn << "\")";
            NativeAction::Function::ACTION_FUNCTION func = [rpn, &fs2020](Event&, sol::state&) {
                execute_rpn(fs2020, rpn.c_str());
            };
            return std::make_shared<NativeAction::Function>(os.str().c_str(), func);
        });
    };
    
    fs2020_table["mfwasm"] = table;
}
