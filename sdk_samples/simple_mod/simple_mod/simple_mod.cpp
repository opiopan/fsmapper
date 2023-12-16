//
// dinputdev.cpp: sample implementation of fsmapper device plugin
//   Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include <mapperplugin.h>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <numbers>
#include <cmath>
#include <limits>

#pragma comment(lib, "fsmappercore.lib")

//============================================================================================
// DLL entry point
//============================================================================================
BOOL APIENTRY DllMain(HMODULE, DWORD reason_for_call, LPVOID){
    switch (reason_for_call){
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//============================================================================================
// Device unit definition
//============================================================================================
#undef max
static FSMDEVUNITDEF unit_def[] ={
    {"x", FSMDU_DIR_INPUT, FSMDU_TYPE_ABSOLUTE, std::numeric_limits<int>::max(), 0},
    {"y", FSMDU_DIR_INPUT, FSMDU_TYPE_ABSOLUTE, std::numeric_limits<int>::max(), 0},
    {"mode", FSMDU_DIR_OUTPUT, FSMDU_TYPE_ABSOLUTE, 1, -1},
};
static constexpr auto unit_x = 0;
static constexpr auto unit_y = 1;
static constexpr auto unit_mode = 2;
static const auto unit_num = sizeof(unit_def) / sizeof(unit_def[0]);

//============================================================================================
// Device representation
//============================================================================================
class device {
public:
    enum class mode{cw, ccw, stop,};

protected:
    static std::unordered_map<uint64_t, std::unique_ptr<device>> devices;
    std::mutex mutex;
    std::condition_variable cv;
    bool should_be_stop{false};
    std::thread emitter;
    FSMAPPER_HANDLE mapper;
    FSMDEVICE device_handle;
    double rpm;
    double radius;
    double angle{0};
    mode direction{mode::stop};
    using clock = std::chrono::steady_clock;
    clock::time_point prev_time;

public:
    static uint64_t create_device(FSMAPPER_HANDLE mapper, FSMDEVICE device_handle, double rpm, double side_length){
        auto new_device = std::make_unique<device>(mapper, device_handle, rpm, side_length);
        auto device_id = reinterpret_cast<uint64_t>(new_device.get());
        devices[device_id] = std::move(new_device);
        return device_id;
    }

    static void close_device(uint64_t device_id){
        devices.erase(device_id);
    }

    static void close_all_devices(){
        devices.clear();
    }

    static device& id_to_device(uint64_t device_id){
        return *devices[device_id];
    }

    device() = delete;
    device(const device&) = delete;
    device& operator = (const device&) = delete;

    device(FSMAPPER_HANDLE mapper, FSMDEVICE device_handle, double rpm, double side_length): 
        mapper(mapper), device_handle(device_handle), rpm(rpm), radius(side_length / 2.){
        emitter = std::move(std::thread([this]{
            std::unique_lock lock{mutex};
            while(true){
                if (direction == mode::stop){
                    cv.wait(lock);
                }else{
                    cv.wait_for(lock, std::chrono::microseconds(200));
                }
                if (should_be_stop){
                    break;
                }
                if (direction != mode::stop){
                    auto now = clock::now();
                    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev_time).count() / 60000.;
                    angle += (direction == mode::cw ? 1. : -1) * 2. * std::numbers::pi * this->rpm * dt;
                    auto x = std::cos(angle) * radius + radius;
                    auto y = std::sin(angle) * radius + radius;
                    prev_time = now;

                    // Issue value change event for the X axis and the Y axis
                    fsmapper_issueEvent(this->mapper, this->device_handle, unit_x, static_cast<int>(std::round(x)));
                    fsmapper_issueEvent(this->mapper, this->device_handle, unit_y, static_cast<int>(std::round(y)));
                }
            }
        }));
    }

    ~device(){
        {
            std::lock_guard lock{mutex};
            should_be_stop = true;
            cv.notify_all();
        }
        emitter.join();
    }

    void change_mode(mode dir){
        std::lock_guard lock{mutex};
        direction = dir;
        prev_time = clock::now();
        cv.notify_all();
    }
};

std::unordered_map<uint64_t, std::unique_ptr<device>> device::devices;

//============================================================================================
// plugin interfaces that expose to fsmapper
//============================================================================================
static bool dev_init(FSMAPPER_HANDLE mapper){
    // The lifespan of the plugin module is from the invocation of dev_init() until dev_term()
    // is called. You can associate the module-scoped context with FSMAPPER_HANDLE, which
    // can be utilized when each function is invoked. 
    // Although this sample module doesn't utilize the module context, here's an example code
    // snippet demonstrating how to associate the context with FSMAPPER_HANDLE.
    fsmapper_setContext(mapper, nullptr);
    return true;
}

static bool dev_term(FSMAPPER_HANDLE mapper){
    // You can also retrieve the module-scope context as shown below
    auto context = fsmapper_getContext(mapper);

    device::close_all_devices();
    return true;
}

static bool dev_open(FSMAPPER_HANDLE mapper, FSMDEVICE dev_handle, LUAVALUE identifier, LUAVALUE options){
    // This function is called every time mapper.device() is invoked in Lua script.
    // It receives the 'identifier' and 'options' parameters from mapper.device(), allowing for device 
    // initialization based on the provided specifications.
    try{
        auto rpm = 20.;
        auto side_length = 4096.;

        auto rpm_value = luav_getItemWithKey(options, "rpm");
        if (!luav_isNull(rpm_value)){
            if (luav_getType(rpm_value) != LV_NUMBER){
                throw std::runtime_error("the value for \"rpm\" option must be numeric");
            }
            rpm = luav_asDouble(rpm_value);
        }
        auto side_length_value = luav_getItemWithKey(options, "side_length");
        if (!luav_isNull(side_length_value)){
            if (luav_getType(side_length_value) != LV_NUMBER){
                throw std::runtime_error("the value for \"side_length\" option must be numeric");
            }
            side_length = luav_asDouble(side_length_value);
        }

        auto device_id = device::create_device(mapper, dev_handle, rpm, side_length);
        fsmapper_setContextForDevice(mapper, dev_handle, reinterpret_cast<void*>(device_id));
        return true;
    }catch (std::runtime_error& err){
        fsmapper_putLog(mapper, FSMLOG_ERROR, err.what());
        return false;
    }
}

static bool dev_start(FSMAPPER_HANDLE mapper, FSMDEVICE dev_handle){
    auto device_id = reinterpret_cast<uint64_t>(fsmapper_getContextForDevice(mapper, dev_handle));
    auto& object = device::id_to_device(device_id);
    object.change_mode(device::mode::cw);
    return true;
}

static bool dev_close(FSMAPPER_HANDLE mapper, FSMDEVICE dev_handle){
    auto device_id = reinterpret_cast<uint64_t>(fsmapper_getContextForDevice(mapper, dev_handle));
    device::close_device(device_id);
    return true;
}

static size_t dev_get_unit_num(FSMAPPER_HANDLE mapper, FSMDEVICE dev_handle){
    return unit_num;
}

static bool dev_get_unit_def(FSMAPPER_HANDLE mapper, FSMDEVICE dev_handle, size_t index, FSMDEVUNITDEF *def){
    if (index >= 0 && index < unit_num){
        *def = unit_def[index];
        return true;
    }else{
        fsmapper_putLog(mapper, FSMLOG_ERROR, "invalid unit id");
        return false;
    }
}

static bool dev_send_unit_value(FSMAPPER_HANDLE mapper, FSMDEVICE dev_handle, size_t index, int value){
    if (index == unit_mode){
        auto device_id = reinterpret_cast<uint64_t>(fsmapper_getContextForDevice(mapper, dev_handle));
        auto& object = device::id_to_device(device_id);
        object.change_mode(value == 0 ? device::mode::stop : value > 0 ? device::mode::cw : device::mode::ccw);
        return true;
    }else{
        fsmapper_putLog(mapper, FSMLOG_ERROR, "invalid unit id");
        return false;
    }
}

//============================================================================================
// plugin entry point
//     fsmapper checks for the existence of this function and inspects the content returned by
//     this function's MAPPER_PLUGIN_DEVICE_OPS object to determine if it's a valid plugin module.
//============================================================================================
static MAPPER_PLUGIN_DEVICE_OPS ops = {
    "rotation", // string to specify in the 'type' parameter of mapper.device() in Lua script
    "SDK sample device that provide rotating coodinates",
    dev_init,
    dev_term,
    dev_open,
    dev_start,
    dev_close,
    dev_get_unit_num,
    dev_get_unit_def,
    dev_send_unit_value,
    nullptr  // this module does not provide the send function that accepts floating point numeric
};

extern "C" DLLEXPORT MAPPER_PLUGIN_DEVICE_OPS *getMapperPluginDeviceOps(){
    return &ops;
}
