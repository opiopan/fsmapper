//
// dcs.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <string>
#include "simhost.h"
#include "action.h"

class DCSWorldSendBuffer;
class DCSObservedData;
class DCSPacket;

class DCSWorld : public SimHostManager::Simulator {
    enum class STATUS{connecting, retrying, connected};

    std::mutex mutex;
    HANDLE schedule_event {nullptr};
    bool should_stop {false};
    std::thread scheduler;
    STATUS status{STATUS::connecting};
    bool is_active {false};
    std::string aircraft_name;
    char rx_buf[16 * 1024];
    std::unique_ptr<DCSWorldSendBuffer> tx_buf;
    std::vector<std::string> chunks;
    std::vector<std::unique_ptr<DCSObservedData>> observed_data_defs;
    HWND representativeWindow{0};

public:
    DCSWorld(SimHostManager &manager, int id);
    DCSWorld(const DCSWorld&) = default;
    DCSWorld(DCSWorld&&) = default;
    virtual ~DCSWorld();
    void initLuaEnv(sol::state &lua) override;
    void changeActivity(bool is_active) override{
        std::unique_lock lock{mutex};
        this->is_active = is_active;
        if (is_active){
            sync_observed_data_definitions(lock);
            sync_chunks(lock);
        }
    }
    HWND getRepresentativeWindow() override {return representativeWindow;}
    mouse_emu::recovery_type getRecoveryType() override {return mouse_emu::recovery_type::none;}

protected:
    size_t process_received_data(std::unique_lock<std::mutex>& lock, char* buf, size_t len, DCSPacket& packet);
    void dispatch_received_command(std::unique_lock<std::mutex>& lock, const DCSPacket& packet);
    void V_command(std::unique_lock<std::mutex>& lock, const DCSPacket& packet);
    void A_command(std::unique_lock<std::mutex>& lock, const DCSPacket& packet);
    void O_command(std::unique_lock<std::mutex>& lock, const DCSPacket& packet);

    void sync_observed_data_definitions(std::unique_lock<std::mutex>& lock);
    void triger_observed_data_event(size_t index, int type, const char* value, size_t length);

    bool send_register_chunk_command_without_lock(uint32_t chunk_id);
    void sync_chunks(std::unique_lock<std::mutex>& lock);
    void send_clear_chunk_command();
    void send_invoke_chunk(uint32_t chunk_id);
    void send_invoke_chunk(uint32_t chunk_id, float argument);
    void send_invoke_chunk(uint32_t chunk_id, const char* argument, size_t length);

    void lua_perform_clickable_action(sol::variadic_args args);
    std::shared_ptr<NativeAction::Function> lua_clickable_action_performer(sol::variadic_args args);
    uint32_t lua_register_chunk(sol::object arg0);
    void lua_clear_chunks();
    void lua_execute_chunk(uint32_t chunk_id, sol::object argument);
    std::shared_ptr<NativeAction::Function> lua_chunk_executer(uint32_t chunk_id, sol::object argument);
    void lua_add_observed_data(sol::object arg0);
    void lua_clear_observed_data();
};
