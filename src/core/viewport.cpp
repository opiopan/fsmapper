//
// viewport.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <stdexcept>
#include <cfenv>
#include <cmath>
#include "viewport.h"
#include "engine.h"
#include "tools.h"
#include "hookdll.h"

//============================================================================================
// Viewport inmplementation
//============================================================================================
ViewPort::ViewPort(ViewPortManager& manager, sol::object def_obj): manager(manager){
    if (def_obj.get_type() != sol::type::table){
        throw MapperException("Viewport definition must be specified as a table");
    }
    auto def = def_obj.as<sol::table>();

    name = std::move(lua_safestring(def["name"]));
    if (name.length() == 0){
        throw MapperException("\"name\" parameter for viewport definition must be specified");
    }

    def_display_no = lua_safevalue<int>(def["displayno"]);
    if (def_display_no && *def_display_no < 1){
        throw  MapperException("\"displayno\" parameter value is invalid, the value must be integer grater than 0");
    }
    is_relative_coordinates = def_display_no ? true : false;
    
    auto&& region_type = lua_safevalue<std::string>(def["region_type"]);
    if (region_type && region_type.value() == "relative"){
        is_relative_coordinates = true;
    }else if (region_type && region_type.value() == "absolute"){
        is_relative_coordinates = false;
    }else if (region_type){
        throw MapperException("Invalid value for \"region_type\" parameter, \"relative\" or \"absolute\" can be specified");
    }
    if (!def_display_no && is_relative_coordinates){
        throw MapperException("region type of viewport should be \"absolute\" if target display number specification is ommited");
    }

    auto x = lua_safevalue<float>(def["x"]);
    auto y = lua_safevalue<float>(def["y"]);
    auto width = lua_safevalue<float>(def["width"]);
    auto height = lua_safevalue<float>(def["height"]);

    if (x && y && width && height){
        def_region = {*x, *y, *width, *height};
    }else if (!is_relative_coordinates || !def_display_no){
        throw MapperException("Viewport region must be specified by \"x\", \"y\", \"width\" and \"height\" parameters, "
                              "if absolute coordinate system is selected or target display number specification is ommited");
    }

    auto&& bgcolor = lua_safevalue<std::string>(def["bgcolor"]);
    if (bgcolor){
        auto color = webcolor_to_colorref(*bgcolor);
        if (color){
            this->bg_color = *color;
        }else{
            std::ostringstream os;
            os << "\"bgcolor\" parameter value is invalid, it should be follow the webcolers format. [";
            os << *bgcolor << "]";
            throw MapperException(std::move(os.str()));
        }
    }
}

ViewPort::~ViewPort(){
    disable();
}

void ViewPort::enable(const std::vector<IntRect> displays){
    if (!def_display_no){
        region.x = std::roundf(def_region.x);
        region.y = std::roundf(def_region.y);
        region.width = std::roundf(std::roundf(def_region.width));
        region.height = std::roundf(def_region.height);
    }else{
        if (displays.size() < *def_display_no){
            std::ostringstream os;
            os << "Display number that is specified as viewport difinition is invalid. [viewport: " << name ;
            os << "] [display: " << *def_display_no << "]";
            throw MapperException(std::move(os.str()));
        }else{
            auto& drect = displays[*def_display_no - 1];
            if (is_relative_coordinates){
                region.x = std::roundf(def_region.x * drect.width + drect.x);
                region.y = std::roundf(def_region.y * drect.height + drect.y);
                region.width = std::roundf(def_region.width * drect.width);
                region.height = std::roundf(def_region.height * drect.height);
            }else{
                region.x = std::roundf(def_region.x + drect.x);
                region.y = std::roundf(def_region.y + drect.y);
                region.width = std::roundf(def_region.width);
                region.height = std::roundf(def_region.height);
            }
        }
    }
    bgwin.start(bg_color, region);
}

void ViewPort::disable(){
    bgwin.stop();
}

int ViewPort::registerView(sol::object def_obj){
    return 0;
}

std::optional<int> ViewPort::getCurrentView(){
    if (views.size() > 0){
        return current_view;
    }else{
        return std::nullopt;
    }
}

void ViewPort::setCurrentView(sol::optional<int> view_no){
    lua_c_interface(manager.get_engine(), "viewport:change_view", [this, &view_no](){
        if (view_no && *view_no >= 0 && view_no < views.size()){
            current_view = *view_no;
        }else{
            throw MapperException("invalid view number is specified");
        }
    });
}

//============================================================================================
// Viewport manager inmplementation
//============================================================================================
ViewPortManager::ViewPortManager(MapperEngine& engine) : engine(engine){
    //hookdll_startGlobalHook(nullptr, nullptr);
}

ViewPortManager::~ViewPortManager(){
    reset_viewports();
    //hookdll_stopGlobalHook();
}

void ViewPortManager::init_scripting_env(sol::table& mapper_table){
    mapper_table.new_usertype<ViewPort>(
        "viewport",
        sol::call_constructor, sol::factories([this](sol::object def){
            return lua_c_interface(this->engine, "mapper.viewport", [this, &def](){
                return this->create_viewvort(def);
            });
        }),
        "current_view", sol::property(&ViewPort::getCurrentView),
        "change_view", &ViewPort::setCurrentView
     );
    mapper_table["start_viewports"] = [this](){
        lua_c_interface(engine, "mapper.start_viewports", [this](){start_viewports();});
    };
    mapper_table["stop_viewports"] = [this](){
        lua_c_interface(engine, "mapper.stop_viewports", [this](){stop_viewports();});
    };
    mapper_table["reset_viewports"] = [this](){
        lua_c_interface(engine, "mapper.reset_viewports", [this](){reset_viewports();});
    };
}

std::shared_ptr<ViewPort> ViewPortManager::create_viewvort(sol::object def_obj){
    std::lock_guard lock(mutex);
    if (status == Status::init){
        auto viewport = std::make_shared<ViewPort>(*this, def_obj);
        viewports.push_back(viewport);
        return viewport;
    }else{
        throw MapperException("Viewport definitions are fixed by calling mapper.start_viewports(). "
                              "You may need to call mapper.reset_viewports() before defining new viewport.");
    }
}

void ViewPortManager::start_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::init || status == Status::suspended){
        if (viewports.size() == 0){
            throw MapperException("no viewports is defined");
        }

        // if (there is one captured windows definition at least){
        //     change_statgus(Status::ready_to_start);
        //     send event to host : MEV_READY_TO_CAPTURE_WINDOW
        //     return;
        // }

        auto prev_status = status;
        change_status(Status::starting);
        lock.unlock();
        try{
            enable_viewport_primitive();
        }catch (MapperException&){
            lock.lock();
            change_status(prev_status);
            throw;
        }
        lock.lock();
        change_status(Status::running);
        lock.unlock();
        engine.sendHostEvent(MEV_START_VIEWPORTS, 0);
    }
}

void ViewPortManager::stop_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::running){
        change_status(Status::suspending);
        lock.unlock();
        try{
            disable_viewport_primitive();
        }catch (MapperException&){
            lock.lock();
            change_status(Status::running);
            throw;
        }
        lock.lock();
        change_status(Status::suspended);
        lock.unlock();
        engine.sendHostEvent(MEV_STOP_VIEWPORTS, 0);
    }
}

void ViewPortManager::reset_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::starting || status == Status::suspending){
        cv.wait(lock, [this](){return status == Status::running || status == Status::suspended;});
    }
    if (status == Status::running){
        change_status(Status::suspending);
        lock.unlock();
        try{
            disable_viewport_primitive();
        }catch (MapperException&){
            lock.lock();
            change_status(Status::running);
            throw;
        }
        lock.lock();
        change_status(Status::suspended);
    }
    if (status != Status::init){
        viewports.clear();
        engine.recommend_gc();
        change_status(Status::init);
        lock.unlock();
        engine.sendHostEvent(MEV_RESET_VIEWPORTS, 0);
    }
}

void ViewPortManager::register_captured_window(uint32_t cwid, HWND hWnd){
}

void ViewPortManager::unregister_captured_window(uint32_t cwid){

}

void ViewPortManager::enable_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::ready_to_start){
        change_status(Status::starting);
        lock.unlock();
        try{
            enable_viewport_primitive();
        }catch (MapperException&){
            lock.lock();
            change_status(Status::ready_to_start);
            throw;
        }
        lock.lock();
        change_status(Status::running);
        lock.unlock();
        engine.sendHostEvent(MEV_START_VIEWPORTS, 0);
    }
}

void ViewPortManager::disable_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::running){
        change_status(Status::suspending);
        lock.unlock();
        try{
            disable_viewport_primitive();
        }catch (MapperException&){
            lock.lock();
            change_status(Status::running);
            throw;
        }
        lock.lock();
        change_status(Status::ready_to_start);
        lock.unlock();
        engine.sendHostEvent(MEV_STOP_VIEWPORTS, 0);
    }
}

void ViewPortManager::enable_viewport_primitive(){
        ::EnumDisplayMonitors(nullptr, nullptr, ViewPortManager::monitor_enum_proc, reinterpret_cast<LPARAM>(this));
        for (auto& viewport : viewports){
            viewport->enable(displays);
        }
}

void ViewPortManager::disable_viewport_primitive(){
        for (auto& viewport : viewports){
            viewport->disable();
        }
}

BOOL ViewPortManager::monitor_enum_proc(HMONITOR hmon, HDC hdc, LPRECT rect, LPARAM context){
    auto self = reinterpret_cast<ViewPortManager*>(context);
    self->displays.emplace_back(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
    return true;
}
