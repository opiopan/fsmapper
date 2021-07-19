//
// pluginapi.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <vector>
#include <sol/sol.hpp>
#include "mapperplugin.h"

class MapperEngine;
class Device;

struct FSMAPPERCTX{
    MapperEngine& engine;
    const char* name;
    void *pluginContext;

    FSMAPPERCTX() = delete;
    FSMAPPERCTX(MapperEngine& engine, const char* name) :
        engine(engine), name(name), pluginContext(nullptr){};
};

struct FSMDEVICECTX{
    Device& device;
    void* pluginContext;
    FSMDEVICECTX() = delete;
    FSMDEVICECTX(Device &dev) : device(dev), pluginContext(nullptr){};
};

struct LUAVALUECTX{
protected:
    sol::object object;
    LVTYPE type;

public:
    LUAVALUECTX();
    LUAVALUECTX(const sol::object& object);
    LUAVALUECTX(const LUAVALUECTX&) = delete;
    LUAVALUECTX(LUAVALUECTX&&) = delete;
    virtual ~LUAVALUECTX() = default;

    LVTYPE getType() const {return type;};
    const sol::object& getObject() const {return object;};
    const char* getStringValue();
    virtual LUAVALUECTX* getItemWithKey(const char* key);
    virtual LUAVALUECTX* getItemWithIndex(size_t index);
};

class LUAVALUE_TABLE : public LUAVALUECTX{
protected:
    std::vector< std::unique_ptr<LUAVALUECTX> > children;

public:
    LUAVALUE_TABLE() = delete;
    LUAVALUE_TABLE(const sol::object& object);
    virtual ~LUAVALUE_TABLE() = default;

    virtual LUAVALUECTX* getItemWithKey(const char* key);
    virtual LUAVALUECTX* getItemWithIndex(size_t index);

protected:
    LUAVALUECTX* newchild(sol::object&& child);
};
