//
// simhost.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>

#include "engine.h"
#include "simhost.h"
#include "fs2020.h"

static const MAPPER_SIM_CONNECTION simkind_dict[] = {MAPPER_SIM_FS2020, MAPPER_SIM_DCS};
static const char* simname_dict[] = {"fs2020", "dcs"};

//============================================================================================
// SimHostManager::Simulator
//    base class of object to represent each flight simulator connection
//============================================================================================
void SimHostManager::Simulator::reportConnectivity(bool connectivity, const char* aircraftname){
    manager.changeConnectivity(id, connectivity, aircraftname);
}


//============================================================================================
// SimHostManager
//    managiment all connection and status with flight simulators
//============================================================================================
SimHostManager::SimHostManager(MapperEngine& engine, uint64_t event_changeAircraft, sol::state& lua): 
    engine(engine), should_stop(false), activeSim(-1), event_changeAircraft(event_changeAircraft){
    //-------------------------------------------------------------------------------
    // initialize Simulator instance correspond to each flight simulator
    //-------------------------------------------------------------------------------
    simulators.push_back(std::make_unique<FS2020>(*this, simulators.size()));
    for (auto& sim: simulators){
        sim->initLuaEnv(lua);
        connectivities.push_back(std::move(Connectivity()));
    }

    //-------------------------------------------------------------------------------
    // create a thread that schedule change aircraft event
    //-------------------------------------------------------------------------------
    scheduler = std::thread([this](){
        std::unique_lock lock(mutex);
        while (true){
            cv.wait(lock,[this](){return this->should_stop || queue.size() > 0;});
            if (this->should_stop){
                break;
            }
            auto msg = std::move(queue.front());
            queue.pop();
            auto oldConnectivity = std::move(connectivities.at(msg.simid));
            auto& newConnectivity = connectivities.at(msg.simid);
            newConnectivity = std::move(msg.connectivity);
            auto oldActiveSim = activeSim;
            if (newConnectivity.isConnected && msg.simid > oldActiveSim){
                activeSim = msg.simid;
            }else if (!newConnectivity.isConnected){
                activeSim = -1;
                for (int i = msg.simid + 1; i < connectivities.size(); i++){
                    if (connectivities[i].isConnected){
                        activeSim = i;
                        break;
                    }
                }
            }

            lock.unlock();
            if (activeSim != oldActiveSim){
                if (activeSim >= 0){
                    simulators[activeSim]->changeActivity(connectivities[activeSim].isConnected);
                }
                if (oldActiveSim >= 0){
                    simulators[oldActiveSim]->changeActivity(connectivities[oldActiveSim].isConnected);
                }
            }
            if (activeSim != oldActiveSim || activeSim == msg.simid){
                std::ostringstream os;
                os << "simhost: ";
                if (activeSim == -1){
                    this->engine.sendEvent(std::move(Event(this->event_changeAircraft, std::move(Event::AssosiativeArray()))));
                    this->engine.sendHostEvent(MEV_CHANGE_SIMCONNECTION, MAPPER_SIM_NONE);
                    this->engine.sendHostEvent(MEV_CHANGE_AIRCRAFT, reinterpret_cast<int64_t>(""));
                    os << "connection with flight simulator has been loast";
                }else{
                    Event event(this->event_changeAircraft, std::move(Event::AssosiativeArray{
                        {"host", std::move(EventValue(simname_dict[activeSim]))},
                        {"aircraft", std::move(EventValue(newConnectivity.aircraftName.c_str()))},
                    }));
                    this->engine.sendEvent(std::move(event));
                    this->engine.sendHostEvent(MEV_CHANGE_SIMCONNECTION, simkind_dict[activeSim]);
                    this->engine.sendHostEvent(MEV_CHANGE_AIRCRAFT, reinterpret_cast<int64_t>(newConnectivity.aircraftName.c_str()));
                    if (newConnectivity.aircraftName.length()){
                        os << "changed aircraft: [sim] " << simname_dict[activeSim];
                        os << ",  [aircraft] " << newConnectivity.aircraftName.c_str();
                    }else{
                        os << "connection with sim has been established: [sim] " << simname_dict[activeSim];
                    }
                }
                this->engine.putLog(MCONSOLE_INFO, os.str());
            }
            lock.lock();
        }
    });
}

SimHostManager::~SimHostManager(){
    {
        std::lock_guard lock(mutex);
        should_stop = true;
        cv.notify_all();
    }
    scheduler.join();
}

MAPPER_SIM_CONNECTION SimHostManager::getConnection(){
    std::lock_guard lock(mutex);
    return activeSim < 0 ? MAPPER_SIM_NONE : simkind_dict[activeSim];
}

std::string SimHostManager::getAircraftName(){
    std::lock_guard lock(mutex);
    return activeSim < 0 ? std::move(std::string()) : connectivities.at(activeSim).aircraftName;
}

void SimHostManager::changeConnectivity(int simid, bool isActive, const char* aircraftName){
    aircraftName = aircraftName ? aircraftName : "";
    std::lock_guard lock(mutex);
    queue.push(std::move(Message(
        simid,
        Connectivity(isActive, std::move(std::string(aircraftName)))
    )));
    cv.notify_all();
}
