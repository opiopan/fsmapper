//
// viewport.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <stdexcept>
#include "viewport.h"
#include "engine.h"
#include "tools.h"

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
}

void ViewPort::enable(){
    auto&& orect = manager.transform_display_to_global(def_display_no, is_relative_coordinates, def_region);
    if (!orect){
        std::ostringstream os;
        os << "Display number that is specified as viewport difinition is invalid. [viewport: " << name ;
        os << "] [display: " << *def_display_no << "]";
    }
    region = *orect;
    bgwin.start(bg_color, *orect);
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
    if (view_no && *view_no >= 0 && view_no < views.size()){
        current_view = *view_no;
    }else{
        throw std::runtime_error("viewport::change_view(): invalid view number is specified");
    }
}

//============================================================================================
// Viewport manager inmplementation
//============================================================================================
ViewPortManager::ViewPortManager(MapperEngine& engine) : engine(engine){
}

ViewPortManager::~ViewPortManager(){
}

void ViewPortManager::init_scripting_env(sol::table& mapper_table){

}

std::optional<IntRect> ViewPortManager::transform_display_to_global(const std::optional<int>& dsiplay, bool is_relative, const FloatRect& rect){
    return std::nullopt;
}

std::shared_ptr<ViewPort> ViewPortManager::create_view_vort(sol::object def_obj){
    return nullptr;
}

void ViewPortManager::start_view_ports(){

}

void ViewPortManager::stop_view_ports(){

}

void ViewPortManager::reset_view_ports(){

}
