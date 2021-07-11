//
// simhidconnection.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <chrono>
#include <sstream>
#include "simhidconnection.h"
#if defined(_WIN64) || defined(_WIN32)
#   include "winserial.h"
    using SerialImp = WinSerial;
#else
#   include "posixserial.h"
    using SerialImp = PosixSerial;
#endif

static const auto INIT_TIMEOUT = std::chrono::seconds(10);

//============================================================================================
//  Recognize device identifier then identify device path
//============================================================================================
std::string SimHIDConnection::identifyDevicePath (LUAVALUE identifier){
    auto path = luav_getItemWithKey(identifier, "path");
    if (path){
        if (luav_getType(path) != LV_STRING){
            throw Exception("\"path\" value for SimHID identifier must be string.");
        }
        auto rc = std::string(luav_asString(path));
        return rc;
    }

#if defined(_WIN64) || defined(_WIN32)
#else
#endif

    throw Exception("No information is specified to identify device path for SimHID device");
}

//============================================================================================
// Commnunication thread imprementation
//============================================================================================
SimHIDConnection::SimHIDConnection(FSMAPPER_HANDLE mapper, SimHID &simhid, const char *devicePath) : 
    mapper(mapper), simhid(simhid), devicePath(devicePath), status(Status::init){
    serial = std::move(std::make_unique<SerialImp>(devicePath));
    simhid_parser_init(&parser, parsedLineBuf, sizeof(parsedLineBuf));
}

void SimHIDConnection::start(){
    //-----------------------------------------------------------------------------
    // create a thread to communicate with SimHID devices
    //-----------------------------------------------------------------------------
    communicator = std::move(std::thread([this] {
        try{
            // send a D command to retrieve device definitions at first
            serial->write("D\r\n"); 

            char buf[256];
            int readlen;
            while((readlen = serial->read(buf, sizeof(buf))) > 0){
                for (int i = 0; i < readlen; i++){
                    if (simhid_parser_parse(&parser, buf[i])){
                        auto cmd = parser.command;
                        if (parser.err){
                            std::string msg = "An error occured during parse data received from SimHID device [";
                            std::ostringstream mout(msg, std::ios_base::app);
                            mout << this->devicePath << "]: " << parser.err;
                            fsmapper_putLog(this->mapper, FSMLOG_ERROR, msg.c_str());
                        }else if (cmd == 'D' || cmd == 'd'){
                            processReceivedData_D();
                        }else if (cmd == 'S' || cmd == 's'){
                            processReceivedData_S();
                        }else if (cmd > 0){
                            std::string msg = "Unexpected data was received from SimHID device[";
                            std::ostringstream mout(msg, std::ios_base::app);
                            mout << this->devicePath << "]";
                            fsmapper_putLog(this->mapper, FSMLOG_WARNING, msg.c_str());
                        }
                    }
                }
            }
        }catch (Exception& e){
            fsmapper_putLog(this->mapper, FSMLOG_ERROR, e.getMessage().c_str());
        }
        std::lock_guard lock(mutex);
        status = Status::stop;
        cv.notify_all();
    }));

    //-----------------------------------------------------------------------------
    // wait until communication thread is initializing
    //-----------------------------------------------------------------------------
    std::unique_lock<std::mutex> lock(mutex);
    if (!cv.wait_for(lock, INIT_TIMEOUT, [this]{return status != Status::init;})){
        std::ostringstream s;
        s << "Initializing SimHID device connected to \"" << devicePath << "\" failed [Timed out]";
        throw Exception(std::move(s.str()));
    };
    if (status != Status::running){
        throw Exception("An error occurred during initializing SimHID device");
    }
}

void SimHIDConnection::processReceivedData_D(){
    // Process D command response
    //     syntax: D [<UnitName> <UnitType> <MinVal> <MaxVal>]
    //     <UnitType>: ABS | REL | OABS | OREL
    std::lock_guard lock(mutex);

    if (status != Status::init){
        std::ostringstream msg;
        msg << "Unexpected data was received from SimHID device[" << devicePath << "]";
        fsmapper_putLog(mapper, FSMLOG_WARNING, msg.str().c_str());
    }else if (parser.paramnum == 0){
        status = Status::running;
        cv.notify_all();
    }else if (parser.paramnum != 4 || !parser.params[2].isNumber || !parser.params[3].isNumber){
        std::ostringstream msg;
        msg << "Received device definition data is unrecognizable format [" << devicePath << "]";
        throw Exception(std::move(msg.str()));
    }else{
        struct DevType {FSMDEVUNIT_DIRECTION dir; FSMDEVUNIT_VALTYPE valtype;};
        static std::map<std::string, DevType> typedict{
            {"ABS", {FSMDU_DIR_INPUT, FSMDU_TYPE_ABSOLUTE}},
            {"REL", {FSMDU_DIR_INPUT, FSMDU_TYPE_RELATIVE}},
            {"OABS", {FSMDU_DIR_OUTPUT, FSMDU_TYPE_ABSOLUTE}},
            {"OREL", {FSMDU_DIR_OUTPUT, FSMDU_TYPE_RELATIVE}},
        };
        std::string name(parser.params[0].strvalue, parser.params[0].len);
        std::string type(parser.params[1].strvalue, parser.params[1].len);
        if (typedict.count(type) == 0){
            std::ostringstream msg;
            msg << "Device unit \"" << name;
            msg << "\" of SimHID device ["<< devicePath << "] cannot be handled";
            fsmapper_putLog(mapper, FSMLOG_WARNING, msg.str().c_str());
        }else{
            UnitDef def = {
                defs.size(),
                name,
                typedict[type].dir,
                typedict[type].valtype,
                parser.params[2].numvalue,
                parser.params[3].numvalue,
            };
            if (def.maxval - def.minval == 1){
                def.valtype = FSMDU_TYPE_BINARY;
            }
            defindex[def.name] = def.index;
            defs.push_back(std::move(def));
        }
    }
}

void SimHIDConnection::processReceivedData_S(){
    if (parser.paramnum != 2 || !parser.params[1].isNumber){
        std::ostringstream msg;
        msg << "Unit value update nortification format from SimHID device [" << devicePath;
        msg << "] cannot be recognized";
        fsmapper_putLog(mapper, FSMLOG_WARNING, msg.str().c_str());
    }
    auto& def = defs[defindex[parser.params[0].strvalue]];
    auto value = parser.params[1].numvalue;
    if (def.index >= 0){
        std::unique_lock lock(mutex);
        for (const auto& device : devices){
            fsmapper_issueEvent(mapper, device.second->getDevice(), def.index, value);
        }
    }
}

//============================================================================================
// Completion synchronization of communication thread
//============================================================================================
SimHIDConnection::~SimHIDConnection(){
    stop();
}

void SimHIDConnection::stop(){
    {
        std::unique_lock<std::mutex> lock(mutex);
        serial->stop();
        cv.wait(lock, [this]{return status == Status::stop || status == Status::joined;});
        if (status == Status::joined){
            // unnecessory to join a commnunicator thrad
            return;
        }
        status = Status::joined;
    }
    communicator.join();
}

//============================================================================================
// Unit definition and unit value operations
//   These operations can be run without lock since unit definitions does not change.
//============================================================================================
size_t SimHIDConnection::getUnitNum(){
    return defs.size();
}

void SimHIDConnection::getUnitDef(size_t index, FSMDEVUNITDEF *def){
    auto& unitdef = defs[index];
    def->name = unitdef.name.c_str();
    def->direction = unitdef.dir;
    def->type = unitdef.valtype;
    def->minValue = unitdef.minval;
    def->maxValue = unitdef.maxval;
}

void SimHIDConnection::sendUnitValue(size_t index, int value){
    auto &unitdef = defs[index];
    std::ostringstream cmd;
    cmd << "S " << unitdef.name << " " << value << "\r\n";
    serial->write(std::move(cmd.str()));
}

//============================================================================================
// Device associated connection operations
//============================================================================================
SimHIDConnection::Device *SimHIDConnection::addDevice(FSMDEVICE device){
    std::lock_guard lock(mutex);
    auto newdev = std::make_unique<Device>(*this, device);
    auto key = newdev.get();
    devices[key] = std::move(newdev);
    return key;
}

void SimHIDConnection::removeDevice(Device *device){
    std::lock_guard lock(mutex);
    devices.erase(device);
}

size_t SimHIDConnection::deviceNum(){
    std::lock_guard lock(mutex);
    return devices.size();
}
