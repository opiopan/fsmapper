//
// engine.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <string>
#include <memory>
#include <sol/sol.hpp>
#include "mappercore.h"
#include "event.h"
#include "device.h"

class DeviceManager;
class DeviceModifier;
class DeviceModifierManager;

class MapperException {
protected:
    std::string message;

public:
    MapperException() = delete;
    MapperException(const char *msg) : message(msg){};
    MapperException(std::string&& msg) : message(std::move(msg)){};
    MapperException(const MapperException&) = default;
    MapperException(MapperException&&) = default;
    virtual ~MapperException() = default;

    virtual MapperException& operator = (const MapperException& v) = default;
    virtual MapperException& operator = (MapperException&&) = default;

    const std::string& getMessage() const {return message;};
};

class MapperEngine {
public:
    typedef std::function<void(MAPPER_EVENT event, int64_t data)> Callback;
    typedef std::function<void(MCONSOLE_MESSAGE_TYPE, const std::string &)> Logger;
    enum class Status{
        init,
        running,
        stop,
        error,
    };

protected : 
    std::mutex mutex;
    Status status;
    Callback callback;
    Logger logger;

    struct {
        std::string scriptPath;
        sol::state lua;
        std::unique_ptr<DeviceManager> deviceManager;
    }scripting;

    struct {
        std::condition_variable cv;
        uint64_t idCounter;
        std::map<uint64_t, std::string> names;
        std::queue< std::unique_ptr<Event> > queue;
    }event;

public:
    MapperEngine(Callback callback, Logger logger);
    virtual ~MapperEngine();

    bool run(std::string&& scriptPath);
    bool stop();
    bool abort();

    Status getStatus(){
        auto lock = std::lock_guard(mutex);
        return status;
    };

    void putLog(MCONSOLE_MESSAGE_TYPE mtype, const std::string& msg){
        logger(mtype, msg);
    };

    uint64_t registerEvent(std::string&& name);
    void unregisterEvent(uint64_t evid);
    void sendEvent(Event&& event);

protected:
    void initScriptingEnvAndRun();
    std::unique_ptr<Event>&& receiveEvent();
};