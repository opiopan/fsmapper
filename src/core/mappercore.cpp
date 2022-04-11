//
// mappercore.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include "mappercore.h"
#include "engine.h"

#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>

//============================================================================================
// MapperContext implementation that hold engine object
//============================================================================================
struct MapperContext{
    std::unique_ptr<MapperEngine> engine;
    MAPPER_CALLBACK_FUNC callbackHandler;
    void* hostContext;
    MAPPER_CONSOLE_HANDLER consoleHandler;

    MapperContext(MAPPER_CALLBACK_FUNC callback, MAPPER_CONSOLE_HANDLER logger, void* hostContext):
        callbackHandler(callback), hostContext(hostContext), consoleHandler(logger){};
    ~MapperContext() = default;

    void callback(MAPPER_EVENT event, int64_t data){
        if (callbackHandler){
            callbackHandler(this, event, data);
        }
    };

    void logger(MCONSOLE_MESSAGE_TYPE type, const std::string &msg){
        if (consoleHandler){
            consoleHandler(this, type, msg.data(), msg.length());
        }
    };
};

//============================================================================================
// mapper core API imprementation
//============================================================================================
DLLEXPORT MapperHandle mapper_init(MAPPER_CALLBACK_FUNC callback, MAPPER_CONSOLE_HANDLER logger, void *hostContext)
{
    auto handle = std::make_unique<MapperContext>(callback, logger, hostContext);
    auto callbackFunctor = std::bind(&MapperContext::callback, handle.get(), std::placeholders::_1, std::placeholders::_2);
    auto loggerFunctor = std::bind(&MapperContext::logger, handle.get(), std::placeholders::_1, std::placeholders::_2);
    handle->engine = std::make_unique<MapperEngine>(callbackFunctor, loggerFunctor);

    return handle.release();
}

DLLEXPORT bool mapper_terminate(MapperHandle handle){
    handle->engine->stop();
    handle->engine->inhibitCallback();
    delete handle;
    return true;
}

DLLEXPORT bool mapper_registerConsoleHandler(MapperHandle handle, MAPPER_CONSOLE_HANDLER func){
    handle->consoleHandler = func;
    return true;
}

DLLEXPORT bool mapper_run(MapperHandle handle, const char *scriptPath){
    return handle->engine->run(std::move(std::string(scriptPath)));
}

DLLEXPORT bool mapper_stop(MapperHandle handle){
    return handle->engine->stop();
}

DLLEXPORT bool mapper_setLogMode(MapperHandle handle, MAPPER_LOGMODE logmode){
    handle->engine->setLogmode(logmode);
    return true;
}

DLLEXPORT void *mapper_getHostContext(MapperHandle handle){
    return handle->hostContext;
}

DLLEXPORT uint32_t mapper_getSimConnection(MapperHandle handle){
    return 0;
}

DLLEXPORT const char* mapper_getAircraftName(MapperHandle handle){
    return nullptr;
}

DLLEXPORT MAPPINGS_STAT mapper_getMappingsStat(MapperHandle handle){
    return handle->engine->get_mapping_stat();
}

DLLEXPORT bool mapper_enumDevices(MapperHandle handle, MAPPER_ENUM_DEVICE_FUNC func, void *context){
    auto&& list = handle->engine->get_device_list();
    for (auto info : list){
        auto rc = func(handle, context, info.class_name, info.device_name.c_str());
        if (!rc){
            return false;
        }
    }
    return true;
}

DLLEXPORT bool mapper_enumCapturedWindows(MapperHandle handle, MAPPER_ENUM_CAPUTURED_WINDOW func, void *context){
    auto&& list = handle->engine->get_captured_window_list();
    for (auto info : list){
        CAPTURED_WINDOW_DEF def = {info.cwid, info.name.c_str(), nullptr, info.is_captured};
        auto rc = func(handle, context, &def);
        if (!rc){
            return false;
        }
    }
    return true;
}

DLLEXPORT bool mapper_enumViewport(MapperHandle handle, MAPPER_ENUM_VIEWPORT_FUNC func, void* context){
    auto&& list = handle->engine->get_viewport_list();
    for (auto viewport : list){
        VIEWPORT_DEF def;
        def.viewport_name = viewport.name.c_str();
        def.viewid = 0;
        for (auto view : viewport.views){
            def.view_name = view.c_str();
            if (!func(handle, context, &def)){
                return false;
            }
            def.viewid++;
        }
    }
    return true;
}

DLLEXPORT bool mapper_captureWindow(MapperHandle handle, uint32_t cwid, HWND hWnd){
    try{
        handle->engine->register_captured_window(cwid, hWnd);
        return true;
    }catch(MapperException& e){
        std::ostringstream os;
        os << "mapper-core: failed to capture window:\n" << e.what();
        handle->engine->putLog(MCONSOLE_WARNING, os.str());
        return false;
    }
}

DLLEXPORT bool mapper_releaseWindw(MapperHandle handle, uint32_t cwid){
    try{
        handle->engine->unregister_captured_window(cwid);
        return true;
    }catch(MapperException& e){
        std::ostringstream os;
        os << "mapper-core: failed to release window:\n" << e.what();
        handle->engine->putLog(MCONSOLE_WARNING, os.str());
        return false;
    }
}

DLLEXPORT bool mapper_startViewPort(MapperHandle handle){
    try{
        return handle->engine->enable_viewports();
    }catch(MapperException& e){
        std::ostringstream os;
        os << "mapper-core: failed to enable viewports:\n" << e.what();
        handle->engine->putLog(MCONSOLE_WARNING, os.str());
        return false;
    }
}

DLLEXPORT bool mapper_stopViewPort(MapperHandle handle){
    try{
        return handle->engine->disable_viewports();
    }catch(MapperException& e){
        std::ostringstream os;
        os << "mapper-core: failed to stop viewports:\n" << e.what();
        handle->engine->putLog(MCONSOLE_WARNING, os.str());
        return false;
    }
}

//============================================================================================
// mapper core tools API imprementation
//============================================================================================
struct MapperToolsContext{
    uint64_t token;
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartupOutput output;
};

DLLEXPORT MapperToolsHandle mapper_tools_init(){
    auto handle = std::make_unique<MapperToolsContext>();
    if (Gdiplus::GdiplusStartup(&handle->token, &handle->input, &handle->output) == Gdiplus::Ok){
        return handle.release();
    }else{
        return nullptr;
    }
}

DLLEXPORT void mapper_tools_terminate(MapperToolsHandle handle){
    if (handle){
        Gdiplus::GdiplusShutdown(handle->token);
        delete handle;
    }
}
