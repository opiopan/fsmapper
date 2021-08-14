//
// device.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <stdexcept>
#include "engine.h"
#include "device.h"
#include "simhid.h"

static const MAPPER_PLUGIN_DEVICE_OPS* builtin_plugins[] = {
    simhid_PluginDeviceOps,
};

//============================================================================================
// Plubin device cupsulized object
//    This object is created each time "mapper.device()" is called in lua script.
//============================================================================================
Device::Device(DeviceClass &deviceClass, std::string &name, const DeviceModifierRule& rule, const sol::object& identifier) : 
    name(name), deviceClass(deviceClass), contextForPlugin(*this){
    std::unique_ptr<LUAVALUECTX> identifier_lua;
    if (identifier.get_type() == sol::type::table){
        identifier_lua = std::make_unique<LUAVALUE_TABLE>(identifier);
    }else{
        identifier_lua = std::make_unique<LUAVALUECTX>(identifier);
    }
    if (!deviceClass.plugin().open(deviceClass, *this, identifier_lua.get())){
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
}

Device::~Device(){
    deviceClass.plugin().close(deviceClass, *this);
}

void Device::issueEvent(size_t unitIndex, int value){
    modifiers[unitIndex]->processUnitValueChangeEvent(value);
}

void Device::sendUnitValue(size_t unitIndex, int value){
    deviceClass.plugin().sendUnitValue(deviceClass, *this, unitIndex, value);
}

//============================================================================================
// Device plugin coupsulized object
//    This object is created correspoinding to each plugin befor running lua script once.
//============================================================================================
DeviceClass::DeviceClass(MapperEngine& engine, const MAPPER_PLUGIN_DEVICE_OPS* pluginOps): 
    pluginOps(pluginOps), contextForPlugin(engine, pluginOps->name){
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
        classes.emplace(plugin->name, std::move(std::make_unique<DeviceClass>(engine, plugin)));
    }
}

//============================================================================================
// Function to create a device that exporse to lua script as name "mapper.device()"
//============================================================================================
class Test{
protected:
    int dev;
public:
    Test() = delete;
    Test(const Test&) = delete;
    Test(Test&&) = delete;
    Test(int dev):dev(dev){
        throw MapperException("Test constractor exception");
    };
    ~Test() = default;
};

sol::object DeviceManager::createDevice(const sol::object &param, sol::this_state s)
{
    sol::state_view lua(s);
    auto out = lua.create_table();

    if (param.get_type() != sol::type::table){
        throw MapperException("Function argument must be a table");
    }
    auto arg = param.as<sol::table>();
    std::string type = arg["type"];
    std::string name = arg["name"];
    sol::object identifire = arg["identifier"];
    sol::object modifiers = arg["modifiers"];
    if (name == ""){
        throw MapperException("Device name as \"name\" parameter must be specified.");
    }
    if (classes.count(type) == 0){
        std::ostringstream os;
        os << "\"type\" parameter value is invalid or no device type is specified. [type: " << type << "]";
        throw MapperException(os.str());
    }
    auto &deviceClass = classes.at(type);
    DeviceModifierRule rule;
    modifierManager.makeRule(modifiers, rule);
    auto device = std::make_shared<Device>(*deviceClass, name, rule, identifire);
    out["_Device"] = device;
    auto unitdefs = device->getUnitDefs();
    for (auto ix_unit = 0; ix_unit < unitdefs.size(); ix_unit++){
        auto& unit = unitdefs[ix_unit];
        auto unit_table = lua.create_table();
        if (unit.direction == FSMDU_DIR_INPUT){
            auto modifier = device->getModifiers()[ix_unit];
            for (auto ix_event = 0; ix_event < modifier->getEventNum(); ix_event++){
                auto event = modifier->getEvent(ix_event);
                unit_table[event.name] = event.id;
            }
        }else{
            unit_table["update"] = [device, ix_unit](int value){
                device->sendUnitValue(ix_unit, value);
            };
        }
        out[unit.name] = unit_table;
    }

    return out;
}
