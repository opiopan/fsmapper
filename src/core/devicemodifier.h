//
// devicemodifier.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <sol/sol.hpp>
#include "mapperplugin.h"

class MapperEngine;

using DEVICEMOD_CLOCK = std::chrono::steady_clock;
using DEVICEMOD_TIME = DEVICEMOD_CLOCK::time_point;
using DEVICEMOD_MILLISEC = std::chrono::milliseconds;

enum class DeviceEvent {
    change,
    down,
    up,
    longpressed,
    singleclick,
    doubleclick,
    increase,
    decrease,
};

class DeviceModifierManager;

class DeviceModifier {
protected:
    DeviceModifierManager& manager;
public:
    struct Event{
        uint64_t id;
        const char* name;
    };

    DeviceModifier() = delete;
    DeviceModifier(DeviceModifierManager& manager) : manager(manager){};
    DeviceModifier(const DeviceModifier&) = default;

    virtual ~DeviceModifier() = default;
    virtual std::shared_ptr<DeviceModifier> makeInstanceFitsToUnit(const char* devname, const FSMDEVUNITDEF& unit) const = 0;
    virtual size_t getEventNum() const = 0;
    virtual Event getEvent(size_t index) const = 0;
    virtual void processUnitValueChangeEvent(int value) = 0;
    virtual void processUnitValueChangeEvent(int value, DEVICEMOD_TIME now){};
    virtual void processTimerEvent(DEVICEMOD_TIME timer_time){};
};

struct DeviceModifierRule {
    std::map<FSMDEVUNIT_VALTYPE, std::shared_ptr<DeviceModifier>> classRule;
    std::map<std::string, std::shared_ptr<DeviceModifier>> unitRule;
    std::shared_ptr<DeviceModifier> raw;

    DeviceModifierRule() = default;
    DeviceModifierRule(const DeviceModifierRule&) = delete;
    DeviceModifierRule(DeviceModifierRule&&) = default;
    ~DeviceModifierRule() = default;

    std::shared_ptr<DeviceModifier> modifierForUnit(const char* devname, const FSMDEVUNITDEF& unit) const{
        if (unitRule.count(unit.name) > 0){
            return unitRule.at(unit.name)->makeInstanceFitsToUnit(devname, unit);
        }else if (classRule.count(unit.type) > 0){
            return classRule.at(unit.type)->makeInstanceFitsToUnit(devname, unit);
        }else{
            return raw->makeInstanceFitsToUnit(devname, unit);
        }
    };
};

class DeviceModifierManager{
protected:
    enum class Status{
        running,
        stopping,
        stop
    };
    struct QueueItem{
        DeviceModifier &modifier;
        int value;
    };

    MapperEngine& engine;
    std::mutex mutex;
    Status status;
    std::condition_variable cv;
    std::queue<QueueItem> event_queue;
    std::map<DEVICEMOD_TIME, DeviceModifier&> timers;
    std::thread scheduler;

public:
    DeviceModifierManager() = delete;
    DeviceModifierManager(MapperEngine& engine);
    DeviceModifierManager(const DeviceModifierManager&) = delete;
    DeviceModifierManager(DeviceModifierManager&&) = delete;
    ~DeviceModifierManager();

    void makeRule(sol::object &def, DeviceModifierRule& rule);

    void stop();

    MapperEngine& getEngine(){return engine;};
    void delegateEventProcessing(DeviceModifier& modifier, int value);
    DEVICEMOD_TIME addTimer(DeviceModifier& modifier, DEVICEMOD_TIME at);
    void cancelTimer(DeviceModifier &modifier, DEVICEMOD_TIME at);
};
