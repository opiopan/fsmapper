//
// simhid.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <map>
#include <string>
#include <mutex>
#include "simhidconnection.h"
#include "simhid.h"

//============================================================================================
// SimHID connection holder object definition
//============================================================================================
class SimHID{
protected: 
    std::mutex mutex;
    FSMAPPER_HANDLE mapper;
    std::map<std::string, std::unique_ptr<SimHIDConnection> > connections;
    std::map<SimHIDConnection::Device*, SimHIDConnection::Device*> devices;

public:
    SimHID(FSMAPPER_HANDLE mapper): mapper(mapper){};
    SimHID() = delete;
    SimHID(const SimHID&) = delete;
    SimHID(SimHID&&) = delete;
    ~SimHID() = default;

    SimHIDConnection::Device* openDevice(FSMDEVICE dev_handle, std::string& devicePath){
        std::lock_guard lock(mutex);
        if (connections.count(devicePath) == 0){
            auto connection = std::make_unique<SimHIDConnection>(mapper, *this, devicePath.c_str());
            connection->start();
            connections.insert(std::make_pair(devicePath, std::move(connection)));
        }
        auto connection = connections.at(devicePath).get();
        auto device_addr = connection->addDevice(dev_handle);
        devices.insert(std::make_pair(device_addr, device_addr));
        return device_addr;
    };

    void closeDevice(SimHIDConnection::Device* device){
        std::lock_guard lock(mutex);
        auto& connection = device->getConnection();
        connection.removeDevice(device);
        if (connection.deviceNum() == 0){
            connections.erase(connection.getDevicePath());
        }
    };
};

//============================================================================================
// plugin interfaces that expose to mapper core
//============================================================================================
static bool simhid_init(FSMAPPER_HANDLE handle){
    try{
        auto simhid = new SimHID(handle);
        fsmapper_setContext(handle, simhid);
    }catch (SimHIDConnection::Exception& e){
        fsmapper_putLog(handle, FSMLOG_ERROR, e.getMessage().c_str());
        return false;
    }
    return true;
}

static bool simhid_term(FSMAPPER_HANDLE handle){
    try{
        auto simhid = static_cast<SimHID*>(fsmapper_getContext(handle));
        delete simhid;
    }catch (SimHIDConnection::Exception& e){
        fsmapper_putLog(handle, FSMLOG_ERROR, e.getMessage().c_str());
        return false;
    }
    return true;
}

static bool simhid_open(FSMAPPER_HANDLE handle, FSMDEVICE device, LUAVALUE identifier, LUAVALUE){
    try{
        auto simhid = static_cast<SimHID*>(fsmapper_getContext(handle));
        auto devPath = SimHIDConnection::identifyDevicePath(identifier);
        auto device_ctx = simhid->openDevice(device, devPath);
        fsmapper_setContextForDevice(handle, device, device_ctx);
    }catch (SimHIDConnection::Exception& e){
        fsmapper_putLog(handle, FSMLOG_ERROR, e.getMessage().c_str());
        return false;
    }
    return true;
}

static bool simhid_start(FSMAPPER_HANDLE handle, FSMDEVICE device){
    return true;
}

static bool simhid_close(FSMAPPER_HANDLE handle, FSMDEVICE device){
    try{
        auto device_ctx = static_cast<SimHIDConnection::Device*>(fsmapper_getContextForDevice(handle, device));
        auto& simhid = device_ctx->getConnection().getSimHID();
        simhid.closeDevice(device_ctx);
    }catch (SimHIDConnection::Exception &e){
        fsmapper_putLog(handle, FSMLOG_ERROR, e.getMessage().c_str());
        return false;
    }
    return true;
}

static size_t simhid_getUnitNum(FSMAPPER_HANDLE handle, FSMDEVICE device){
    auto device_ctx = static_cast<SimHIDConnection::Device*>(fsmapper_getContextForDevice(handle, device));
    return device_ctx->getConnection().getUnitNum();
}

static bool simhid_getUnitDef(FSMAPPER_HANDLE handle, FSMDEVICE device, size_t index, FSMDEVUNITDEF *def){
    auto device_ctx = static_cast<SimHIDConnection::Device*>(fsmapper_getContextForDevice(handle, device));
    device_ctx->getConnection().getUnitDef(index, def);
    return true;
}

static bool simhid_sendUnitValue(FSMAPPER_HANDLE handle, FSMDEVICE device, size_t index, int value){
    auto device_ctx = static_cast<SimHIDConnection::Device*>(fsmapper_getContextForDevice(handle, device));
    device_ctx->getConnection().sendUnitValue(index, value);
    return true;
}

static MAPPER_PLUGIN_DEVICE_OPS simhid_ops = {
    "simhid",
    "SimHID device",
    simhid_init,
    simhid_term,
    simhid_open,
    simhid_start,
    simhid_close,
    simhid_getUnitNum,
    simhid_getUnitDef,
    simhid_sendUnitValue,
};

MAPPER_PLUGIN_DEVICE_OPS* simhid_PluginDeviceOps = &simhid_ops;
