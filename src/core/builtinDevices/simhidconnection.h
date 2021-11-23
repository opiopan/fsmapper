//
// simhidconnection.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "mapperplugin.h"
#include "simhidparser.h"

class SimHID;

class SimHIDConnection{
public:
    class Exception{
    protected:
        std::string message;

    public:
        Exception() = delete;
        Exception(const char *msg) : message(msg){};
        Exception(std::string&& msg) : message(std::move(msg)){};
        Exception(const Exception&) = default;
        Exception(Exception&&) = default;
        virtual ~Exception() = default;

        virtual Exception& operator = (const Exception& v) = default;
        virtual Exception& operator = (Exception&&) = default;

        const std::string& getMessage() const {return message;};
    };

    class Device{
    protected:
        SimHIDConnection &connection;
        FSMDEVICE device;

    public:
        Device() = delete;
        Device(SimHIDConnection &connection, FSMDEVICE device) :
            connection(connection), device(device){};
        Device(const Device &) = delete;
        Device(Device &&) = delete;
        ~Device() = default;
        SimHIDConnection& getConnection() {return connection;};
        FSMDEVICE getDevice() const { return device; };
    };

    class SerialComm{
    public:
        virtual ~SerialComm() = default;
        virtual int read(void* buf, int len) = 0;
        virtual void write(std::string&& data) = 0;
        virtual void stop() = 0;
    };

protected:
    std::mutex mutex;
    std::condition_variable cv;
    std::thread communicator;
    FSMAPPER_HANDLE mapper;
    SimHID& simhid;
    std::string devicePath;
    enum class Status{
        init,
        running,
        stop,
        joined,
    } status;
    std::unique_ptr<SerialComm> serial;
    struct DeviceId{
        std::string key;
        std::string value;
    };
    std::vector<DeviceId> deviceid;
    struct UnitDef{
        UnitDef():index(-1){};
        UnitDef(size_t index, std::string& name , 
                FSMDEVUNIT_DIRECTION dir, FSMDEVUNIT_VALTYPE type, int min, int max) :
                index(index), name(name), dir(dir), valtype(type), minval(min), maxval(max){};
        UnitDef(UnitDef&) = delete;
        UnitDef(UnitDef&&) = default;
        size_t index;
        std::string name;
        FSMDEVUNIT_DIRECTION dir;
        FSMDEVUNIT_VALTYPE valtype;
        int minval;
        int maxval;
    };
    std::vector<UnitDef> defs;
    std::map<std::string, size_t> defindex;
    std::map<Device*, std::unique_ptr<Device> > devices;
    SimhidParserCtx parser;
    char parsedLineBuf[256];

public:
    static std::string identifyDevicePath(LUAVALUE identifier);

    SimHIDConnection(FSMAPPER_HANDLE mapper, SimHID& simhid, const char* devicePath);
    SimHIDConnection() = delete;
    SimHIDConnection(const SimHIDConnection&) = delete;
    SimHIDConnection(SimHIDConnection&&) = delete;
    ~SimHIDConnection();

    const std::string& getDevicePath() const{return devicePath;};
    FSMAPPER_HANDLE getMapper(){return mapper;};
    SimHID& getSimHID(){return simhid;};

    size_t getUnitNum();
    void getUnitDef(size_t index, FSMDEVUNITDEF* def);
    void sendUnitValue(size_t index, int value);

    void start();
    void stop();

    Device* addDevice(FSMDEVICE device);
    void removeDevice(Device* device);
    size_t deviceNum();

protected:
    void processReceivedData_I();
    void processReceivedData_D();
    void processReceivedData_S();
};
