//
// simhost.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>
#include <sol/sol.hpp>
#include "mappercore.h"

class MapperEngine;

class SimHostManager {
public:
    class Simulator{
    protected:
        SimHostManager& manager;
        int id;
    public:
        Simulator(SimHostManager& manager, int id) : manager(manager), id(id){};
        Simulator(const Simulator&) = default;
        Simulator(Simulator&&) = default;
        virtual ~Simulator() = default;
        virtual void initLuaEnv(sol::state& lua) = 0;
        virtual void changeActivity(bool isActive) = 0;
    protected:
        void reportConnectivity(bool connectivity, const char* aircraftname);
        SimHostManager& getManager(){return manager;};
    };

protected:
    MapperEngine& engine;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread scheduler;
    bool should_stop;
    std::vector<std::unique_ptr<Simulator>> simulators;
    struct Connectivity{
        bool isConnected;
        std::string aircraftName;

        Connectivity() : isConnected(false){};
        Connectivity(bool isConnected, std::string&& aircraftName): isConnected(isConnected), aircraftName(std::move(aircraftName)){};
        Connectivity(const Connectivity&) = default;
        Connectivity(Connectivity&&) = default;
        ~Connectivity() = default;
        Connectivity& operator=(Connectivity&& src){
            isConnected = src.isConnected;
            aircraftName = std::move(src.aircraftName);
            return *this;
        };
    };
    struct Message{
        int simid;
        Connectivity connectivity;

        Message() = delete;
        Message(int simid, Connectivity&& connectivity): simid(simid), connectivity(std::move(connectivity)){};
        Message(const Message&) = default;
        Message(Message&&) = default;
        ~Message() = default;
    };
    std::queue<Message> queue;
    std::vector<Connectivity> connectivities;
    int activeSim;
    uint64_t event_changeAircraft;

public:
    SimHostManager(MapperEngine& engine, uint64_t event_changeAircraft, sol::state& lua);
    SimHostManager(const SimHostManager&) = delete;
    SimHostManager(SimHostManager&&) = delete;
    ~SimHostManager();

    // functions for engine
    MAPPER_SIM_CONNECTION getConnection();
    std::string getAircraftName();

    // fuctions for each Simulator instance
    MapperEngine& getEngine(){return engine;};
    void changeConnectivity(int simid, bool isActive, const char* aircraftName);
};