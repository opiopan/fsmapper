//
// dinputdev.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <stdexcept>
#include <sstream>
#include <dinput.h>
#include "dinputdev.h"
#include "tools.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//============================================================================================
// Capsulize DirectInputDevice8 interface
//============================================================================================
class DirectInputDevice{
public:
    class MapperDevice{
    protected:
        DirectInputDevice& dinput_device;
        FSMDEVICE mapper_device;
    public:
        MapperDevice() = delete;
        MapperDevice(DirectInputDevice& dinput_device, FSMDEVICE mapper_device) : 
            dinput_device(dinput_device), mapper_device(mapper_device){}
        MapperDevice(const MapperDevice&) = delete;
        MapperDevice(MapperDevice&&) = delete;
        MapperDevice& operator = (const MapperDevice&) = delete;
        MapperDevice& operator = (MapperDevice&&) = delete;
        ~MapperDevice() = default;
        DirectInputDevice& get_dinput_device(){return dinput_device;}
        FSMDEVICE get_mapper_device(){return mapper_device;}
    };

protected:
    std::string name;
    FSMAPPER_HANDLE mapper;
    ComPtr<IDirectInputDevice8A> dinput_device;
    std::unordered_map<MapperDevice*, std::unique_ptr<MapperDevice>> mapper_devices;
    std::vector<std::string> object_names;

public:
    DirectInputDevice() = delete;
    DirectInputDevice(const std::string& name, FSMAPPER_HANDLE mapper, IDirectInputDevice8A* dinput_device) :
        name(name), mapper(mapper), dinput_device(dinput_device){
        DIDEVCAPS caps;
        caps.dwSize = sizeof(caps);
        auto hr = dinput_device->GetCapabilities(&caps);
        hr = dinput_device->EnumObjects(enum_object_callback, this, DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
        std::ostringstream os;
        os << "dinput device [" << name << "] has " << object_names.size() << " objects:";
        for (auto& oname : object_names){
            os << std::endl << "    " << oname;
        }
        fsmapper_putLog(mapper, FSMLOG_DEBUG, os.str().c_str());
    }
    ~DirectInputDevice() = default;
    DirectInputDevice(const DirectInputDevice&) = delete;
    DirectInputDevice(DirectInputDevice&&) = delete;
    DirectInputDevice& operator = (const DirectInputDevice&) = delete;
    DirectInputDevice& operator = (DirectInputDevice&&) = delete;

    const std::string& get_name(){return name;}

    MapperDevice* add_mapper_device(FSMDEVICE device_handle){
        auto device = std::make_unique<MapperDevice>(*this, device_handle);
        auto device_ptr = device.get();
        mapper_devices.emplace(device_ptr, std::move(device));
        return device_ptr;
    }

    void remove_mapper_device(MapperDevice* device){
        mapper_devices.erase(device);
    }

    size_t get_mapper_device_num(){
        return mapper_devices.size();
    }

protected:
    static BOOL enum_object_callback(LPCDIDEVICEOBJECTINSTANCEA lpddoi, LPVOID pvRef){
        auto self = static_cast<DirectInputDevice*>(pvRef);
        self->object_names.push_back(lpddoi->tszName);
        return true;
    }
};

//============================================================================================
// Capsulize DirectInput8 interface
//============================================================================================
class DirectInput{
protected:
    ComPtr<IDirectInput8A> dinput;
    std::vector<DIDEVICEINSTANCEA> device_ids;

public:
    DirectInput(){
    	HMODULE hModule = nullptr;
	    ::GetModuleHandleExA(
		    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
		    reinterpret_cast<LPCSTR>(dinput_PluginDeviceOps),
		    &hModule);
        IDirectInput8A* rawptr = nullptr;
        auto hr = DirectInput8Create(hModule, DIRECTINPUT_VERSION, IID_IDirectInput8A, 
                                     reinterpret_cast<void**>(&rawptr), nullptr);
        if (!SUCCEEDED(hr)){
            throw std::runtime_error("failed to initialize DirectInput environment");
        }
        dinput = rawptr;

        IDirectInputDevice8A* rawdev = nullptr;
    }
    ~DirectInput() = default;
    DirectInput(const DirectInput&) = delete;
    DirectInput(DirectInput&&) = delete;
    DirectInput& operator = (const DirectInput&) = delete;
    DirectInput& operator = (DirectInput&&) = delete;

    operator IDirectInput8A* () const {return dinput;}
    IDirectInput8A* operator -> () const {return dinput;}

    const std::vector<DIDEVICEINSTANCEA>& get_ids() const {return device_ids;}
    void reflesh_ids(){
        device_ids.clear();
        dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, enum_devices_callback, this, DIEDFL_ALLDEVICES);
    }

    std::unique_ptr<DirectInputDevice> create_device(FSMAPPER_HANDLE mapper, const std::string& name){
        reflesh_ids();
        for (auto& id : device_ids){
            if (name == id.tszInstanceName){
                IDirectInputDevice8A* rawdev = nullptr;
                auto hr = dinput->CreateDevice(id.guidInstance, &rawdev, nullptr);
                if (!SUCCEEDED(hr)){
                    throw std::runtime_error("failed to create a direct input devce");
                }
                return std::move(std::make_unique<DirectInputDevice>(name, mapper, rawdev));
            }
        }
        throw std::runtime_error("specified direct input device is not found");
    }

protected:
    static BOOL enum_devices_callback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef){
        auto self = reinterpret_cast<DirectInput*>(pvRef);
        self->device_ids.push_back(*lpddi);
        return true;
    }
};

//============================================================================================
// Base context object to handle DirectInput device
//============================================================================================
class DinputDev{
protected:
    std::mutex mutex;
    bool should_stop;
    std::thread poller;
    FSMAPPER_HANDLE mapper;
    DirectInput dinput;
    std::unordered_map<std::string, std::unique_ptr<DirectInputDevice>> dinput_devices;
    std::unordered_map<DirectInputDevice::MapperDevice*, DirectInputDevice::MapperDevice*> mapper_devices;
    
public:
    DinputDev(FSMAPPER_HANDLE mapper) : mapper(mapper){
        dinput.reflesh_ids();
        std::ostringstream os;
        os << "detected " << dinput.get_ids().size() << " devices:";
        for (auto& id : dinput.get_ids()){
            os << std::endl << "    " << id.tszInstanceName;
        }
        fsmapper_putLog(mapper, FSMLOG_DEBUG, os.str().c_str());
    }

    ~DinputDev(){
        stop();
    }

    void stop(){
        std::unique_lock lock(mutex);
    }

    DirectInputDevice::MapperDevice* open_device(FSMDEVICE dev_handle, const std::string& name){
        if (dinput_devices.count(name) == 0){
            dinput_devices.emplace(name, std::move(dinput.create_device(mapper, name)));
        }
        auto& dinput_device = dinput_devices.at(name);
        auto mapper_device = dinput_device->add_mapper_device(dev_handle);
        mapper_devices.emplace(mapper_device, mapper_device);
        return mapper_device;
    }

    void close_device(DirectInputDevice::MapperDevice* mapper_device){
        auto& dinput_device = mapper_device->get_dinput_device();
        dinput_device.remove_mapper_device(mapper_device);
        if (dinput_device.get_mapper_device_num() == 0){
            dinput_devices.erase(dinput_device.get_name());
        }
    }

protected:
};

//============================================================================================
// plugin interfaces that expose to mapper core
//============================================================================================
template <typename TFunc, typename TReturn>
inline auto plugin_interface(FSMAPPER_HANDLE handle, TReturn value_in_fail, TFunc function){
    try{
        return function();
    }catch (std::runtime_error& e){
        fsmapper_putLog(handle, FSMLOG_ERROR, e.what());
        return value_in_fail;
    }
}

static bool dinputdev_init(FSMAPPER_HANDLE handle){
    return plugin_interface(handle, false, [handle](){
        auto dinputdev = new DinputDev(handle);
        fsmapper_setContext(handle, dinputdev);
        return true;
    });
}

static bool dinputdev_term(FSMAPPER_HANDLE handle){
    return plugin_interface(handle, false, [handle](){
        auto dinputdev = static_cast<DinputDev*>(fsmapper_getContext(handle));
        delete dinputdev;
        return true;
    });
}

static bool dinputdev_open(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle, LUAVALUE identifier)
{
    return plugin_interface(handle, false, [handle, dev_handle, identifier](){
        auto dinputdev = static_cast<DinputDev*>(fsmapper_getContext(handle));
        auto name = luav_getItemWithKey(identifier, "name");
        if (name){
            if (luav_getType(name) != LV_STRING){
                throw std::runtime_error("\"name\" value for direct input device identifier must be string.");
            }
            auto device = dinputdev->open_device(dev_handle, luav_asString(name));
            fsmapper_setContextForDevice(handle, dev_handle, device);
            return true;
        }
        throw std::runtime_error("\"name\" parameter must be specified for direct input device identifier");
    });
}

static bool dinputdev_close(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle){
    return plugin_interface(handle, false, [handle, dev_handle](){
        auto dinputdev = static_cast<DinputDev*>(fsmapper_getContext(handle));
        auto device = static_cast<DirectInputDevice::MapperDevice*>(fsmapper_getContextForDevice(handle, dev_handle));
        dinputdev->close_device(device);
        return true;
    });
}

static size_t dinputdev_getUnitNum(FSMAPPER_HANDLE handle, FSMDEVICE device){
    return 0;
}

static bool dinputdev_getUnitDef(FSMAPPER_HANDLE handle, FSMDEVICE device, size_t index, FSMDEVUNITDEF *def){
    return true;
}

static bool dinputdev_sendUnitValue(FSMAPPER_HANDLE handle, FSMDEVICE device, size_t index, int value){
    return true;
}

static MAPPER_PLUGIN_DEVICE_OPS dinputdev_ops = {
    "dinput",
    dinputdev_init,
    dinputdev_term,
    dinputdev_open,
    dinputdev_close,
    dinputdev_getUnitNum,
    dinputdev_getUnitDef,
    dinputdev_sendUnitValue,
};

MAPPER_PLUGIN_DEVICE_OPS* dinput_PluginDeviceOps = &dinputdev_ops;
