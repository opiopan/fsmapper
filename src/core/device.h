//
// device.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <vector>
#include <map>
#include <sol/sol.hpp>
#include "pluginapi.h"
#include "devicemodifier.h"

class DeviceClass;
class DeviceManager;
class MaaperEngine;

class Device{
    std::string name;
    MapperEngine& engine;
    DeviceClass& deviceClass;
    FSMDEVICECTX contextForPlugin;
    std::vector<FSMDEVUNITDEF> unitDefs;
    std::vector< std::shared_ptr<DeviceModifier> > modifiers;

public:
    Device() = delete;
    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device(MapperEngine& engine, DeviceClass& deviceClass, std::string& name,
           const DeviceModifierRule& rule, const sol::object& identifier, const sol::object& options);
    virtual ~Device();

    operator FSMDEVICECTX* (){return &contextForPlugin;};
    const std::vector<FSMDEVUNITDEF>& getUnitDefs() const {return unitDefs;};
    const std::vector<std::shared_ptr<DeviceModifier>>& getModifiers() const {return modifiers;};

    void issueEvent(size_t unitIndex, int value);
    void sendUnitValue(size_t unitIndex, int value);

    sol::object create_event_table(sol::this_state s);
    sol::object create_upstream_id_table(sol::this_state s);
};

class DeviceClass{
protected:
    DeviceManager& manager;
    const MAPPER_PLUGIN_DEVICE_OPS* pluginOps;
    FSMAPPERCTX contextForPlugin;

public:
    DeviceClass() = delete;
    DeviceClass(const DeviceClass&) = delete;
    DeviceClass(DeviceClass&&) = delete;
    DeviceClass(MapperEngine& engine, DeviceManager& manager, const MAPPER_PLUGIN_DEVICE_OPS* pluginOps);
    ~DeviceClass();

    operator FSMAPPERCTX* (){return &contextForPlugin;};
    const MAPPER_PLUGIN_DEVICE_OPS& plugin(){return *pluginOps;};
    DeviceManager& get_manager(){return manager;}
};

class DeviceManager{
public:
    struct DeviceInfo{
        const char* device_class;
        const Device* device;
        DeviceInfo(const char* devclass, const Device* object) : device_class(devclass), device(object){}
        DeviceInfo(const DeviceInfo& src) : device_class(src.device_class), device(src.device){}
        DeviceInfo& operator = (const DeviceInfo& src){
            device_class = src.device_class;
            device = src.device;
            return *this;
        }
    };
protected:
    MapperEngine& engine;
    DeviceModifierManager modifierManager;
    std::map<std::string, std::unique_ptr<DeviceClass>> classes;
    std::map<std::string, DeviceInfo> ids;

public:    
    DeviceManager(MapperEngine& engine);
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager(DeviceManager&&) = delete;
    ~DeviceManager() = default;

    std::shared_ptr<Device> createDevice(const sol::object &param);
    void removeDevice(const char* name);

    const std::map<std::string, DeviceInfo>& getDeviceInfor(){return ids;}

    void init_scripting_env(sol::table& mapper_table);
};
