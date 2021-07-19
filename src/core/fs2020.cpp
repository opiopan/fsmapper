//
// fs2020.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <chrono>
#include "engine.h"
#include "fs2020.h"

static const auto CONNECTING_INTERVAL = std::chrono::milliseconds(500);
static const auto EVENT_SIM_START = 1;
static const auto DINAMIC_EVENT_MIN = 100;

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
                ::std::this_thread::sleep_for(CONNECTING_INTERVAL);
                lock.lock();
                if (shouldStop){
                    return;
                }
            }
            status = Status::connected;

            // Request an event when the simulation starts
            SimConnect_SubscribeToSystemEvent(simconnect, EVENT_SIM_START, "SimStart");

            //-------------------------------------------------------------------------------
            // process SimConnect events
            //-------------------------------------------------------------------------------
            while(status != Status::disconnected){
                if (shouldStop){
                    return;
                }
                lock.unlock();
                auto eventix = ::WaitForMultipleObjects(2, events, false, INFINITE);
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
                                status = Status::start;
                                lock.unlock();
                                this->reportConnectivity(true, nullptr);
                                lock.lock();
                            }
                        }else if (pData->dwID == SIMCONNECT_RECV_ID_QUIT){
                            status = Status::disconnected;
                            lock.unlock();
                            this->reportConnectivity(false, nullptr);
                            lock.lock();
                        }
                    }
                }
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

}
