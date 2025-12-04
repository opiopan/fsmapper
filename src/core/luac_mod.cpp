//
// luac_mod.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "luac_mod.h"
#include "mapperplugin_luac.h"
#include "engine.h"

#include <memory>
#include <string>
#include <list>
#include <format>

struct _FSMAPPER_LUAC_CTX {};

struct fsmapper_luac_ctx : public _FSMAPPER_LUAC_CTX {
    using ptr = std::shared_ptr<fsmapper_luac_ctx>;
    using list = std::list<ptr>;
    using list_iterator = list::iterator;

    std::string module_name;
    list::iterator self;

    fsmapper_luac_ctx(const char* mod_name) : module_name(mod_name ? mod_name : ""){}
    fsmapper_luac_ctx(const fsmapper_luac_ctx&) = delete;
    fsmapper_luac_ctx& operator=(const fsmapper_luac_ctx&) = delete;
    fsmapper_luac_ctx(fsmapper_luac_ctx&&) = delete;
    fsmapper_luac_ctx& operator=(const fsmapper_luac_ctx&&) = delete;
};

static fsmapper_luac_ctx::list contexts;

DLLEXPORT FSMAPPER_LUAC_CTX fsmapper_luac_open_ctx(lua_State *L, const char* mod_name){
    sol::state_view lua{L};
    if (lua["mapper"].get_type() == sol::type::table){
        auto ctx = std::make_shared<fsmapper_luac_ctx>(mod_name);
        contexts.push_back(ctx);
        ctx->self = std::prev(contexts.end());
        return ctx.get();
    }else{
        return nullptr;
    }
}

DLLEXPORT void fsmapper_luac_release_ctx(FSMAPPER_LUAC_CTX ctx){
    auto tctx = static_cast<fsmapper_luac_ctx *>(ctx);
    contexts.erase(tctx->self);
}

DLLEXPORT void fsmapper_luac_putLog(FSMAPPER_LUAC_CTX ctx, FSMAPPER_LOG_TYPE type, const char *msg){
    auto tctx = static_cast<fsmapper_luac_ctx*>(ctx);
    auto engine = mapper_EngineInstance();
    MCONSOLE_MESSAGE_TYPE mtype;
    switch (type){
    case FSMLOG_ERROR:
        mtype = MCONSOLE_ERROR;
        break;
    case FSMLOG_WARNING:
        mtype = MCONSOLE_WARNING;
        break;
    case FSMLOG_INFO:
        mtype = MCONSOLE_INFO;
        break;
    case FSMLOG_MESSAGE:
        mtype = MCONSOLE_MESSAGE;
        break;
    case FSMLOG_DEBUG:
        mtype = MCONSOLE_DEBUG;
        break;
    default:
        mtype = MCONSOLE_MESSAGE;
        break;
    }
    engine->putLog(mtype, std::format("{}: {}", tctx->module_name, msg).c_str());
}

DLLEXPORT void fsmapper_luac_send_event(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid){
    auto engine = mapper_EngineInstance();
    engine->sendEvent(Event{evid});
}

DLLEXPORT void fsmapper_luac_send_event_int(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid, long long data){
    auto engine = mapper_EngineInstance();
    engine->sendEvent(Event{evid, static_cast<int64_t>(data)});
}

DLLEXPORT void fsmapper_luac_send_event_float(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid, double data){
    auto engine = mapper_EngineInstance();
    engine->sendEvent(Event{evid, data});
}

DLLEXPORT void fsmapper_luac_send_event_str(FSMAPPER_LUAC_CTX ctx, FSMAPPER_EVENT_ID evid, const char *data){
    auto engine = mapper_EngineInstance();
    engine->sendEvent(Event{evid, std::move(std::string(data))});
}

struct _FSMAPPER_LUAC_ASYNC_SOURCE {};
namespace {
    struct luac_async_source : public _FSMAPPER_LUAC_ASYNC_SOURCE{
        using list = std::list<luac_async_source>;
        using list_iterator = list::iterator;

        std::shared_ptr<fsmapper_luac_ctx> ctx;
        lua_CFunction event_provider;
        list_iterator self;
        bool signaled{false};

        luac_async_source(std::shared_ptr<fsmapper_luac_ctx> ctx, lua_CFunction function) : ctx(ctx), event_provider(function){}
        luac_async_source(const luac_async_source&) = delete;
        luac_async_source& operator=(const luac_async_source&) = delete;
        luac_async_source(luac_async_source&&) = delete;
        luac_async_source& operator=(const luac_async_source&&) = delete;
    };

    static luac_async_source::list async_sources;
}

DLLEXPORT FSMAPPER_LUAC_ASYNC_SOURCE fsmapper_luac_create_async_source(FSMAPPER_LUAC_CTX ctx, lua_CFunction event_provider){
    auto tctx = static_cast<fsmapper_luac_ctx *>(ctx);
    async_sources.emplace_back(*tctx->self, event_provider);
    auto source = std::prev(async_sources.end());
    source->self = source;
    return &*source;
}

DLLEXPORT void fsmapper_luac_release_async_source(FSMAPPER_LUAC_ASYNC_SOURCE source){
    auto tsource = static_cast<luac_async_source *>(source);
    async_sources.erase(tsource->self);
}

DLLEXPORT void fsmapper_luac_async_source_signal(FSMAPPER_LUAC_ASYNC_SOURCE source){
    auto tsource = static_cast<luac_async_source *>(source);
    auto engine = mapper_EngineInstance();
    engine->notify_luacmod_event(source);
}

namespace luac_mod{
    bool mark_async_source_signaled(FSMAPPER_LUAC_ASYNC_SOURCE source){
        auto tsource = static_cast<luac_async_source *>(source);
        if (!tsource->signaled){
            tsource->signaled = true;
            return true;
        }else{
            return false;
        }
    }

    static void send_event(FSMAPPER_EVENT_ID evid, sol::object& object){
        Event event{evid, std::move(object)};
        auto engine = mapper_EngineInstance();
        engine->sendEventNoLock(std::move(event));
    }

    void dispatch_async_events(std::unique_lock<std::mutex>& lock){
        auto engine = mapper_EngineInstance();
        auto& lua = engine->getLuaState();
        lua_State* L = lua.lua_state();
        for (auto& source : async_sources){
            if (source.signaled){
                source.signaled = false;

                lua_pushcfunction(L, source.event_provider);
                sol::protected_function event_provider(lua, sol::stack_reference(L, -1));
                lua_pop(L, 1);
                
                lock.unlock();
                sol::protected_function_result result = event_provider();
                lock.lock();

                if (!result.valid()) {
                    sol::error err = result.get<sol::error>();
                    auto msg = std::format(
                        "luac_mod: Lua C module [{}] returned an error during async event processing:\n{}", 
                        source.ctx->module_name, err.what());
                    engine->putLog(MCONSOLE_WARNING, msg.c_str());
                }
                else {
                    if (result.return_count() == 0){
                        // No event
                    }else if (result.return_count() >= 2){
                        // Single event
                        auto maybe_evid = result[0].as<std::optional<FSMAPPER_EVENT_ID>>();
                        sol::object value = result[1];
                        if (maybe_evid.has_value()){
                            send_event(*maybe_evid, value);
                        }else{
                            auto msg = std::format(
                                "luac_mod: Lua C module [{}] did not return valid event id during async event processing.",
                                source.ctx->module_name);
                            engine->putLog(MCONSOLE_WARNING, msg.c_str());  
                        }
                    }else if (result.return_count() == 1){
                        // Multiple event
                        sol::object table_obj = result[0];
                        if (table_obj.get_type() == sol::type::table){
                            sol::table table = table_obj;
                            bool invalid_data{false};
                            for (auto i = 1; i <= table.size(); i++){
                                sol::object event_obj = table[i];
                                if (event_obj.get_type() == sol::type::table){
                                    sol::table event_table = event_obj;
                                    auto maybe_evid = static_cast<sol::object>(event_table["evid"]).as<std::optional<FSMAPPER_EVENT_ID>>();
                                    if (maybe_evid.has_value()){
                                        sol::object value = event_table["value"];
                                        send_event(*maybe_evid, value);
                                    }else{
                                        invalid_data;
                                    }
                                }
                            }
                            if (invalid_data){
                                auto msg = std::format(
                                    "luac_mod: Lua C module [{}] did not return valid event id during async event processing.",
                                    source.ctx->module_name);
                                engine->putLog(MCONSOLE_WARNING, msg.c_str());  
                            }
                        }else{
                            auto msg = std::format(
                                "luac_mod: Lua C module [{}] did not return a table during async event processing.",
                                source.ctx->module_name);
                            engine->putLog(MCONSOLE_WARNING, msg.c_str());  
                        }
                    }
                }                
            }
        }
    }

    void cleanup_async_sources(){
        async_sources.clear();
    }
}