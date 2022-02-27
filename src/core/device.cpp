//
// device.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <stdexcept>
#include "engine.h"
#include "device.h"
#include "simhid.h"
#include "dinputdev.h"

static const MAPPER_PLUGIN_DEVICE_OPS* builtin_plugins[] = {
    simhid_PluginDeviceOps,
    dinput_PluginDeviceOps,
};

//============================================================================================
// Plubin device cupsulized object
//    This object is created each time "mapper.device()" is called in lua script.
//============================================================================================
Device::Device(MapperEngine& engine, DeviceClass &deviceClass, std::string &name,
               const DeviceModifierRule& rule, const sol::object& identifier, const sol::object& options) : 
    name(name), engine(engine), deviceClass(deviceClass), contextForPlugin(*this){
    std::unique_ptr<LUAVALUECTX> identifier_lua;
    if (identifier.get_type() == sol::type::table){
        identifier_lua = std::make_unique<LUAVALUE_TABLE>(identifier);
    }else{
        identifier_lua = std::make_unique<LUAVALUECTX>(identifier);
    }
    std::unique_ptr<LUAVALUECTX> options_lua;
    if (options.get_type() == sol::type::table){
        options_lua = std::make_unique<LUAVALUE_TABLE>(options);
    }else{
        options_lua = std::make_unique<LUAVALUECTX>(options);
    }
    if (!deviceClass.plugin().open(deviceClass, *this, identifier_lua.get(), options_lua.get())){
        std::ostringstream os;
        os << "failed to open a device: [name: " << name << "] [type: " << deviceClass.plugin().name << "]";
        throw MapperException(os.str());
    }
    auto unitnum = deviceClass.plugin().getUnitNum(deviceClass, *this);
    for (int i = 0; i < unitnum; i++){
        FSMDEVUNITDEF def;
        deviceClass.plugin().getUnitDef(deviceClass, *this, i, &def);
        unitDefs.push_back(def);
        if (def.direction == FSMDU_DIR_INPUT){
            modifiers.push_back(rule.modifierForUnit(this->name.c_str(), def));
        }else{
            modifiers.push_back(nullptr);
        }
    }
    if (!deviceClass.plugin().start(deviceClass, *this)){
        std::ostringstream os;
        os << "failed to start a device: [name: " << name << "] [type: " << deviceClass.plugin().name << "]";
        throw MapperException(os.str());
    }
    engine.notifyUpdate(engine.UPDATED_DEVICES);
}

Device::~Device(){
    deviceClass.get_manager().removeDevice(name.c_str());
    deviceClass.plugin().close(deviceClass, *this);
    engine.notifyUpdate(engine.UPDATED_DEVICES);
}

void Device::issueEvent(size_t unitIndex, int value){
    modifiers[unitIndex]->processUnitValueChangeEvent(value);
}

void Device::sendUnitValue(size_t unitIndex, int value){
    lua_c_interface(engine, "device:send", [this, unitIndex, value](){
        if (unitDefs.size() <= unitIndex || unitDefs[unitIndex].direction != FSMDU_DIR_OUTPUT){
            throw MapperException("invalid upstream id");
        }
        deviceClass.plugin().sendUnitValue(deviceClass, *this, unitIndex, value);
    });
}

sol::object Device::create_event_table(sol::this_state s){
    sol::state_view lua(s);
    auto out = lua.create_table();
    for (auto ix_unit = 0; ix_unit < unitDefs.size(); ix_unit++){
        auto& unit = unitDefs[ix_unit];
        if (unit.direction == FSMDU_DIR_INPUT){
            auto unit_table = lua.create_table();
            auto modifier = modifiers[ix_unit];
            for (auto ix_event = 0; ix_event < modifier->getEventNum(); ix_event++){
                auto event = modifier->getEvent(ix_event);
                unit_table[event.name] = event.id;
            }
            out[unit.name] = unit_table;
        }
    }
    return out;
}

sol::object Device::create_upstream_id_table(sol::this_state s){
    sol::state_view lua(s);
    auto out = lua.create_table();
    for (auto ix_unit = 0; ix_unit < unitDefs.size(); ix_unit++){
        auto& unit = unitDefs[ix_unit];
        out[unit.name] = ix_unit;
    }
    return out;
}

//============================================================================================
// Device plugin coupsulized object
//    This object is created correspoinding to each plugin befor running lua script once.
//============================================================================================
DeviceClass::DeviceClass(MapperEngine& engine, DeviceManager& manager, const MAPPER_PLUGIN_DEVICE_OPS* pluginOps): 
    manager(manager), pluginOps(pluginOps), contextForPlugin(engine, pluginOps->name){
    if (!pluginOps->init(*this)){
        std::ostringstream os;
        os << "failed to initalize device plugin: [plugin name: " << pluginOps->name << "]" << std::endl;
        throw MapperException(os.str());
    }
}

DeviceClass::~DeviceClass(){
    pluginOps->term(*this);
}

//============================================================================================
// Device plugin manager
//============================================================================================
DeviceManager::DeviceManager(MapperEngine& engine): engine(engine), modifierManager(engine){
    for (int i = 0; i < sizeof(builtin_plugins) / sizeof(builtin_plugins[0]); i++){
        auto plugin = builtin_plugins[i];
        auto device_class = std::make_unique<DeviceClass>(engine, *this, plugin);
        classes.emplace(plugin->name, std::move(device_class));
    }
}

//============================================================================================
// Function to create a device that exporse to lua script as name "mapper.device()"
//============================================================================================
std::shared_ptr<Device> DeviceManager::createDevice(const sol::object &param){
    if (param.get_type() != sol::type::table){
        throw MapperException("Function argument must be a table");
    }
    auto arg = param.as<sol::table>();
    std::string type = arg["type"];
    std::string name = arg["name"];
    sol::object identifire = arg["identifier"];
    sol::object modifiers = arg["modifiers"];
    sol::object options = arg["options"];
    if (name == ""){
        throw MapperException("Device name as \"name\" parameter must be specified.");
    }
    if (ids.count(name)){
        std::ostringstream os;
        os << "Device name \"" << name << "\" is already used.";
        throw MapperException(os.str());
    }
    if (classes.count(type) == 0){
        std::ostringstream os;
        os << "\"type\" parameter value is invalid or no device type is specified. [type: " << type << "]";
        throw MapperException(os.str());
    }
    auto &deviceClass = classes.at(type);
    DeviceModifierRule rule;
    modifierManager.makeRule(modifiers, rule);
    auto device = std::make_shared<Device>(engine, *deviceClass, name, rule, identifire, options);
    ids.emplace(name, std::move(DeviceInfo(deviceClass->plugin().name, device.get())));
    return device;
}

void DeviceManager::removeDevice(const char* name){
    ids.erase(name);
}


//============================================================================================
// Define lua user type which is created when "mapper.device()" is called
//============================================================================================
void DeviceManager::init_scripting_env(sol::table& mapper_table){
    mapper_table.new_usertype<Device>(
        "device",
        sol::call_constructor, sol::factories([this](sol::object def){
            return lua_c_interface(engine, "mapper.device", [this, &def](){
                return createDevice(def);
            });
        }),
        "events", sol::property(&Device::create_event_table),
        "upstream_ids", sol::property(&Device::create_upstream_id_table),
        "send", &Device::sendUnitValue
    );
}
