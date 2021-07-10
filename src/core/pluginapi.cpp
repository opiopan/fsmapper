//
// pluginapi.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <vector>
#include <sstream>
#include <sol/sol.hpp>
#include "engine.h"
#include "device.h"
#include "pluginapi.h"

//============================================================================================
// Funcitons to interuct with fsmapper
//============================================================================================
DLLEXPORT void fsmapper_putLog(FSMAPPER_HANDLE mapper, FSMAPPER_LOG_TYPE type, const char *msg){
    std::ostringstream os;
    os << mapper->name << ": " << msg;
    mapper->engine.putLog(static_cast<MCONSOLE_MESSAGE_TYPE>(type), os.str());
}

DLLEXPORT void fsmapper_abort(FSMAPPER_HANDLE mapper){
    mapper->engine.abort();
}

DLLEXPORT void fsmapper_setContext(FSMAPPER_HANDLE mapper, void *context){
    mapper->pluginContext = context;
}

DLLEXPORT void *fsmapper_getContext(FSMAPPER_HANDLE mapper){
    return mapper->pluginContext;
}

DLLEXPORT void fsmapper_setContextForDevice(FSMAPPER_HANDLE mapper, FSMDEVICE device, void *context){
    device->pluginContext = context;
}

DLLEXPORT void *fsmapper_getContextForDevice(FSMAPPER_HANDLE mapper, FSMDEVICE device){
    return device->pluginContext;
}

DLLEXPORT void fsmapper_issueEvent(FSMAPPER_HANDLE mapper, FSMDEVICE device, int index, int value){
    device->device.issueEvent(index, value);
}

//============================================================================================
// LUA value accessor
//============================================================================================
LUAVALUECTX::LUAVALUECTX() : type(LV_NULL){};

LUAVALUECTX::LUAVALUECTX(sol::object&& object): object(std::move(object)){
    switch (object.get_type()){
    case sol::type::lua_nil:
        type = LV_NULL;
        break;
    case sol::type::boolean:
        type = LV_BOOL;
        break;
    case sol::type::number:
        type = LV_NUMBER;
        break;
    case sol::type::string:
        type = LV_STRING;
        break;
    case sol::type::table:
        type = LV_TABLE;
        break;
    default:
        type = LV_OTHERS;
        break;
    }
}
const char* LUAVALUECTX::getStringValue(){
    return object.as<const char*>();
}

LUAVALUECTX* LUAVALUECTX::getItemWithKey(const char* key){
    return nullptr;
}

LUAVALUECTX *LUAVALUECTX::getItemWithIndex(size_t index){
    return nullptr;
}

LUAVALUE_TABLE::LUAVALUE_TABLE(sol::object&& object) : LUAVALUECTX(std::move(object)){
}

LUAVALUECTX* LUAVALUE_TABLE::getItemWithKey(const char *key){
    sol::object child = object.as<sol::table>()[key];
    return newchild(std::move(child));
}

LUAVALUECTX* LUAVALUE_TABLE::getItemWithIndex(size_t index){
    sol::object child = object.as<sol::table>()[index];
    return newchild(std::move(child));
};

LUAVALUECTX* LUAVALUE_TABLE::newchild(sol::object &&child){
    auto type = child.get_type();
    if (type == sol::type::lua_nil){
        return nullptr;
    }
    else if (type == sol::type::table){
        children.push_back(std::make_unique<LUAVALUE_TABLE>(std::move(child)));
        return children.back().get();
    }else{
        children.push_back(std::make_unique<LUAVALUECTX>(std::move(child)));
        return children.back().get();
    }
}

LVTYPE luav_getType(LUAVALUE lv){
    return lv->getType();
}

bool luav_isNull(LUAVALUE lv){
    return lv->getType() == LV_NULL;
}

bool luav_asBool(LUAVALUE lv){
    switch (lv->getType()){
    case LV_BOOL:
        return lv->getObject().as<bool>();
    case LV_NUMBER:
        return lv->getObject().as<double>() != 0.;
    case LV_STRING:
        return lv->getObject().as<std::string>().length() != 0;
    default:
        return false;
    }
}

int64_t luav_asInt(LUAVALUE lv){
    switch (lv->getType()){
    case LV_BOOL:
        return static_cast<int64_t>(lv->getObject().as<bool>());
    case LV_NUMBER:
        return lv->getObject().as<int64_t>();
    default:
        return 0;
    }
}

double luav_asDouble(LUAVALUE lv){
    switch (lv->getType()){
    case LV_BOOL:
        return static_cast<double>(lv->getObject().as<bool>());
    case LV_NUMBER:
        return lv->getObject().as<double>();
    default:
        return 0;
    }
}

const char* luav_asString(LUAVALUE lv){
    switch (lv->getType()){
    case LV_STRING:
        return lv->getStringValue();
    default:
        return "";
    }
}

LUAVALUE luav_getItemWithKey(LUAVALUE lv, const char* key){
    return lv->getItemWithKey(key);
}

LUAVALUE luav_getItemWithIndex(LUAVALUE lv, size_t index){
    return lv->getItemWithIndex(index);
}
