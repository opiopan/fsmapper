//
// fs2020.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <map>
#include <windows.h>
#include <SimConnect.h>
#include "simhost.h"
#include "tools.h"

class FS2020 : public SimHostManager::Simulator{
protected:
    class SimConnectHandle{
    protected:
        HANDLE handle = nullptr;
    public:
        SimConnectHandle() = default;
        SimConnectHandle(HANDLE handle):handle(handle){};
        SimConnectHandle(const SimConnectHandle&) = delete;
        SimConnectHandle(SimConnectHandle&&) = delete;
        ~SimConnectHandle(){if (handle){SimConnect_Close(handle);}};
        SimConnectHandle& operator = (SimConnectHandle&) = delete;
        SimConnectHandle& operator = (SimConnectHandle&&) = delete;
        SimConnectHandle& operator = (HANDLE handle){
            if (this->handle){
                SimConnect_Close(this->handle);
            }
            this->handle = handle;
            return *this;
        };
        HANDLE get_handle()const {return handle;};
        operator HANDLE()const {return handle;};
    };

    enum class Status{
        connecting,
        connected,
        start,
        disconnected,
    };

    std::mutex mutex;
    bool shouldStop = false;    
    Status status = Status::connecting;
    bool isActive = false;
    std::string aircraftName;
    std::thread scheduler;

    SimConnectHandle simconnect;
    WinHandle event_simconnect;
    WinHandle event_interrupt;
    std::map<std::string, SIMCONNECT_CLIENT_EVENT_ID> sim_events;

public:
    FS2020(SimHostManager& manager, int id);
    FS2020(const FS2020&) = default;
    FS2020(FS2020&&) = default;
    virtual ~FS2020();
    virtual void initLuaEnv(sol::state& lua);
    virtual void changeActivity(bool isActive);

protected:
    SIMCONNECT_CLIENT_EVENT_ID getSimEventId(const std::string& event_name);
    void sendSimEventId(SIMCONNECT_CLIENT_EVENT_ID eventid);
};