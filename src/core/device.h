//
// device.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <vector>
#include <map>
#include <sol/sol.hpp>
#include "engine.h"
#include "pluginapi.h"
#include "devicemodifier.h"

class DeviceClass;

class Device{
    std::string name;
    DeviceClass& deviceClass;
    FSMDEVICECTX contextForPlugin;
    std::vector<FSMDEVUNITDEF> unitDefs;
    std::vector< std::shared_ptr<DeviceModifier> > modifiers;

public:
    Device() = delete;
    Device(Device&) = delete;
    Device(Device&&) = delete;
    Device(DeviceClass &deviceClass, std::string &name, const DeviceModifierRule& rule, sol::object&& identifier);
    virtual ~Device();

    operator FSMDEVICECTX* (){return &contextForPlugin;};
    const std::vector<FSMDEVUNITDEF>& getUnitDefs() const {return unitDefs;};
    const std::vector<std::shared_ptr<DeviceModifier>>& getModifiers() const {return modifiers;};

    void issueEvent(size_t unitIndex, int value);
    void sendUnitValue(size_t unitIndex, int value);
};

class DeviceClass{
protected:
    const MAPPER_PLUGIN_DEVICE_OPS* pluginOps;
    FSMAPPERCTX contextForPlugin;

public:
    DeviceClass() = delete;
    DeviceClass(DeviceClass&) = delete;
    DeviceClass(DeviceClass&&) = delete;
    DeviceClass(MapperEngine& engine, const MAPPER_PLUGIN_DEVICE_OPS* pluginOps);
    ~DeviceClass();

    operator FSMAPPERCTX* (){return &contextForPlugin;};
    const MAPPER_PLUGIN_DEVICE_OPS& plugin(){return *pluginOps;};
};

class DeviceManager{
protected:
    MapperEngine& engine;
    DeviceModifierManager modifierManager;
    std::map<std::string, std::unique_ptr<DeviceClass>> classes;

public:    
    DeviceManager(MapperEngine& engine);
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager(DeviceManager&&) = delete;
    ~DeviceManager() = default;

    sol::object createDevice(const sol::object &param, sol::this_state s);
};
