//
// windowcapture.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "windowcapture.h"
#include "composition.h"
#include "engine.h"
#include "viewport.h"

#include <optional>

//============================================================================================
// Utility functions
//============================================================================================
std::optional<FloatRect> parese_rect_def(sol::object object){
    if (object.get_type() == sol::type::table){
        sol::table def = object;
        auto x = lua_safevalue<float>(def["x"]);
        auto y = lua_safevalue<float>(def["y"]);
        auto width = lua_safevalue<float>(def["width"]);
        auto height = lua_safevalue<float>(def["height"]);
        FloatRect rc{
            x ? *x : 0, 
            y ? *y : 0,
            width ? *width : 0, 
            height ? *height : 0};
        if (rc.width != 0, height != 0){
            return rc;
        }
    }
    return std::nullopt;
}

//============================================================================================
// Window image streamer
//============================================================================================
namespace capture{
    class image_streamer_imp : public image_streamer{
    protected:
        ViewPortManager& manager;
        uint32_t id;
        std::string name{"DCS World"};
        std::vector<std::string> target_titles;
        HWND hwnd{nullptr};
        FloatRect capture_rect{0, 0, 0, 0};
        CComPtr<IDXGISwapChain1> swap_chain;

    public:
        image_streamer_imp(ViewPortManager&manager, sol::object def_obj, uint32_t id): manager(manager), id(id){
            if (def_obj.get_type() != sol::type::table){
                throw MapperException("function argument is not a table");
            }
            auto def = def_obj.as<sol::table>();
            auto name = lua_safevalue<std::string>(def["name"]);
            if (name && name->size() > 0){
                this->name = std::move(*name);
            }
            sol::object titles_obj = def["window_titles"];
            if (titles_obj.get_type() == sol::type::table){
                sol::table titles = titles_obj;
                for (int i = 1; i <= titles.size(); i++){
                    auto title = lua_safevalue<std::string>(titles[i]);
                    if (title && title->size() > 0){
                        target_titles.emplace_back(std::move(*title));
                    }else{
                        throw MapperException("each element of 'window_titles' have to be specified as a string");
                    }
                }
            }else if (titles_obj.valid()){
                throw MapperException("'window_titles' parameter for captured window definition must be a table");
            }
            auto title = lua_safestring(def["window_title"]);
            if (title.size() > 0){
                target_titles.emplace_back(std::move(title));
            }
            if (target_titles.size() == 0){
                target_titles.emplace_back("Digital Combat Simulator");
            }

            auto&& rect = parese_rect_def(def["capture_rect"]);
            if (rect){
                capture_rect = *rect;
            }
        }

        uint32_t get_id() const override{return id;}
        const char* get_name() const override{return name.c_str();}
        HWND get_hwnd() const override{return hwnd;}
        const std::vector<std::string>& get_target_titles() const override{return target_titles;}

        void set_hwnd(HWND handle) override{
            if (!swap_chain){
                hwnd = handle;
            }
        }

        void start_capture() override{

        }

        void stop_capture() override{

        }

        void dispose() override{

        }
    };

    std::shared_ptr<image_streamer> create_image_streamer(ViewPortManager&manager, uint32_t id, sol::object arg){
        return std::make_shared<image_streamer_imp>(manager, arg, id);
    }
}

//============================================================================================
// Initialize Lua scripting environment
//
//  Note: 
//  The environment for image_streamer is initialized in the start up procedure of Viewport
//  manager. Refer 'viewport.cpp'
//============================================================================================
namespace capture{
    void init_scripting_env(sol::table& mapper_table){
    }
}
