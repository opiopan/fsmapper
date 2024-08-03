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
    char tx_buf[16 * 1024];

public:
    DCSWorld(SimHostManager &manager, int id);
    DCSWorld(const DCSWorld&) = default;
    DCSWorld(DCSWorld&&) = default;
    virtual ~DCSWorld();
    void initLuaEnv(sol::state &lua) override;
    void changeActivity(bool is_active) override{
        std::lock_guard lock{mutex};
        this->is_active = is_active;
    }
    HWND getRepresentativeWindow() override {
        return nullptr;
    }

protected:
    void process_received_data(std::unique_lock<std::mutex>& lock, const char* buf, int len, std::string& context);
    void dispatch_received_command(std::unique_lock<std::mutex> &lock, const char *cmd, int len);
};
