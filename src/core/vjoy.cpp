//
// vjoy.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma comment(lib, "vJoyInterface.lib")

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <sol/sol.hpp>
#include <windows.h>
#include <public.h>
#include <vjoyinterface.h>

#include "vjoy.h"
#include "tools.h"
#include "engine.h"
#include "dinputdev.h"
#include "action.h"

class vJoyDevice;

//============================================================================================
// Class representing operable objects of vJoy device such as button
//============================================================================================
class vJoyDeviceUnit{
public:
    using UPDATER = std::function<void (int64_t)>;
protected:
    vJoyDevice& device;
    std::string name;
    UPDATER updater;

public:
    vJoyDeviceUnit(vJoyDevice& device, std::string&& name, UPDATER updater) : device(device), name(std::move(name)), updater(updater){}
    ~vJoyDeviceUnit() = default;

    void update(int64_t value){
        updater(value);
    }

    void setValue(sol::object object){
        auto value = lua_safevalue<int64_t>(object);
        if (object.get_type() == sol::type::boolean){
            value = (object.as<bool>() ? 1 :0);
        }
        if (value){
            update(*value);
        }
    }

    std::shared_ptr<NativeAction::Function> valueSetter(sol::object o_defvalue);
};

//============================================================================================
// Class representing vJoy device that is created when lua function "vjoy()" is called
//============================================================================================
class vJoyDevice{
protected:
    vJoyManager& manager;
    UINT device_id;
    std::unordered_map<std::string, std::shared_ptr<vJoyDeviceUnit>> axes;
    std::vector<std::shared_ptr<vJoyDeviceUnit>> buttons;
    std::vector<std::shared_ptr<vJoyDeviceUnit>> povs;

public:
    static std::shared_ptr<vJoyDevice> create(vJoyManager& manager, sol::object arg){
        auto device_id = ::lua_safevalue<int>(arg);
        if (!device_id || *device_id < 1){
            throw std::runtime_error("vJoy device id must be specified as integer grater than 0");
        }

        return std::make_shared<vJoyDevice>(manager, *device_id);
    }

    vJoyDevice(vJoyManager& manager, int device_id): manager(manager), device_id(device_id){
        //
        // initialize vJoy
        //
        if (!vJoyEnabled()){
            throw std::runtime_error("getting vJoy attributes failed");
        }
        if (!::AcquireVJD(device_id)){
            std::ostringstream os;
            os << "vJoy device " << device_id;
            auto status = ::GetVJDStatus(device_id);
            if (status == VJD_STAT_OWN){
                os << " is already opend";
            }else if (status == VJD_STAT_BUSY){
                os << " is already owned by another feeder";
            }else if (VJD_STAT_MISS){
                os << " is not installed or is disabled";
            }else{
                os << " cannot be acquired";
            }
            throw std::runtime_error(std::move(os.str()));
        }
        ResetVJD(device_id);

        //
        // genereate objects corresponds to each axis;
        //
        static const struct {int usage; const char* name;} axis_defs[] = {
            {HID_USAGE_X, "x"},
            {HID_USAGE_Y, "y"},
            {HID_USAGE_Z, "z"},
            {HID_USAGE_RX, "rx"},
            {HID_USAGE_RY, "ry"},
            {HID_USAGE_RZ, "rz"},
            {HID_USAGE_SL0, "slider1"},
            {HID_USAGE_SL1, "slider2"},
            {0, nullptr},
        };
        for (auto def = axis_defs; def->name; def++){
            if (GetVJDAxisExist(device_id, def->usage)){
                LONG axis_max, axis_min;
                GetVJDAxisMax(device_id, def->usage, &axis_max);
                GetVJDAxisMin(device_id, def->usage, &axis_min);
                constexpr auto val_range = JOYSTICK_AXIS_VALUE_MAX - JOYSTICK_AXIS_VALUE_MIN;
                const auto dev_range = axis_max - axis_min;
                constexpr auto val_bias = JOYSTICK_AXIS_VALUE_MIN;
                const auto dev_bias = axis_min;
                axes.emplace(
                    std::move(std::string(def->name)),
                    std::make_shared<vJoyDeviceUnit>(
                        *this,
                        std::move(std::string(def->name)), 
                        [this, def, val_range, dev_range, val_bias, dev_bias](int64_t value){
                            auto axisval = (value - val_bias) * dev_range / val_range + dev_bias;
                            SetAxis(axisval, this->device_id, def->usage);
                        }));
            }
        }

        //
        // genereate objects corresponds to each button;
        //
        for (int idx = 0; idx < GetVJDButtonNumber(device_id); idx++){
            std::ostringstream os;
            os << "button" << idx;
            buttons.push_back(std::make_shared<vJoyDeviceUnit>(*this, std::move(os.str()), [this, idx](int64_t value){
                SetBtn(value, this->device_id, idx);
            }));
        }

        //
        // genereate objects corresponds to each disc POV;
        //
        for (int idx = 0; idx < GetVJDDiscPovNumber(device_id); idx++){
            std::ostringstream os;
            os << "pov" << idx;
            povs.push_back(std::make_shared<vJoyDeviceUnit>(*this, std::move(os.str()), [this, idx](int64_t value){
                SetDiscPov(value, this->device_id, idx);
            }));
        }

        //
        // genereate objects corresponds to each disc POV;
        //
        for (int idx = 0; idx < GetVJDContPovNumber(device_id); idx++){
            std::ostringstream os;
            os << "pov" << idx;
            povs.push_back(std::make_shared<vJoyDeviceUnit>(*this, std::move(os.str()), [this, idx](int64_t value){
                SetContPov(value, this->device_id, idx);
            }));
        }

        //
        // logging
        //
        std::ostringstream os;
        os << "vjoy: vJoy device #" << device_id << " has " << axes.size() + buttons.size() + povs.size() << " objects";
        if (axes.size()){
            os << std::endl << "    " << axes.size() << " axes: ";
            for (auto& [name, axis] : axes){
                os << name << " ";
            }
        }
        if (buttons.size()){
            os << std::endl << "    " << buttons.size() << " buttons";
        }
        if (povs.size()){
            os << std::endl << "    " << povs.size() << " POVs";
        }
        manager.getEngine().putLog(MCONSOLE_DEBUG, os.str());
    }

    ~vJoyDevice(){
        RelinquishVJD(device_id);
    };

    MapperEngine& getEngine(){return manager.getEngine();}
    UINT getDeviceId(){return device_id;}

    std::shared_ptr<vJoyDeviceUnit> getAxis(sol::object o_name){
        auto name = lua_safestring(o_name);
        if (axes.count(name) > 0){
            return axes.at(name);
        }else{
            return nullptr;
        }
    }

    std::shared_ptr<vJoyDeviceUnit> getButton(sol::object o_idx){
        auto idx = lua_safevalue<int64_t>(o_idx);
        if (idx && *idx > 0 && *idx <= buttons.size()){
            return buttons[*idx];
        }else{
            return nullptr;
        }
    }

    std::shared_ptr<vJoyDeviceUnit> getPov(sol::object o_idx){
        auto idx = lua_safevalue<int64_t>(o_idx);
        if (idx && *idx > 0 && *idx <= povs.size()){
            return povs[*idx];
        }else{
            return nullptr;
        }
    }

};

//============================================================================================
// Functions of vJoyDeviceUnit that cannot be implement before vJoyDevice definition
// since C++ does not allow forward reference
//============================================================================================
std::shared_ptr<NativeAction::Function> vJoyDeviceUnit::valueSetter(sol::object o_defvalue){
    auto defvalue = lua_safevalue<int64_t>(o_defvalue);
    std::ostringstream os;
    os << "vjoy#" << device.getDeviceId() << "." << name << ":set_value(";
    if (defvalue){
        os << *defvalue;
    }else if (o_defvalue.get_type() == sol::type::boolean){
        defvalue = (o_defvalue.as<bool>() ? 1 : 0);
        os << *defvalue;
    }
    os << ")";
    if (defvalue){
        NativeAction::Function::ACTION_FUNCTION func = [this, defvalue](Event&, sol::state&){
            this->update(*defvalue);
        };
        return std::make_shared<NativeAction::Function>(os.str().c_str(), func);
    }else{
        NativeAction::Function::ACTION_FUNCTION func = [this, defvalue](Event& event, sol::state&){
            auto type = event.getType();
            if (type == Event::Type::bool_value){
                this->update(event.getAs<bool>() ? 1 : 0);
            }else if (type == Event::Type::int_value || type == Event::Type::double_value){
                this->update(event.getAs<int64_t>());
            }
        };
        return std::make_shared<NativeAction::Function>(os.str().c_str(), func);
    }
}

//============================================================================================
// Build lua environment for vjoy usertype
//============================================================================================
vJoyManager::vJoyManager(MapperEngine& engine) : engine(engine){
}

vJoyManager::~vJoyManager(){
}

void vJoyManager::init_scripting_env(sol::state& lua, sol::table& mapper_table){
    mapper_table.new_usertype<vJoyDevice>(
        "virtual_joystick",
        sol::call_constructor, sol::factories([this](sol::object arg){
            return lua_c_interface(engine, "vjoy", [this, &arg]{
                return vJoyDevice::create(*this, arg);
            });
        }),
        "get_axis", &vJoyDevice::getAxis,
        "get_button", &vJoyDevice::getButton,
        "get_pov", &vJoyDevice::getPov
    );

    mapper_table.new_usertype<vJoyDeviceUnit>(
        "_virtual_joystick_unit",
        "new", sol::no_constructor,
        "set_value", &vJoyDeviceUnit::setValue,
        "value_setter", &vJoyDeviceUnit::valueSetter
    );
}
