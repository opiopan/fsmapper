//
// capturedwindow.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "capturedwindow.h"
#include "engine.h"
#include "tools.h"
#include "hookdll.h"

CapturedWindow::CapturedWindow(MapperEngine& engine, uint32_t cwid, sol::object& def_obj) : engine(engine){
    if (def_obj.get_type() != sol::type::table){
        throw MapperException("function argument is not a table");
    }
    auto def = def_obj.as<sol::table>();
    auto name = lua_safevalue<std::string>(def["name"]);
    if (name && name->size() > 0){
        this->name = std::move(*name);
    }else{
        throw MapperException("valid name is not specified for the captured window");
    }
    auto omit_system_region = lua_safevalue<bool>(def["omit_system_region"]);
    auto fix_touch_issue = lua_safevalue<bool>(def["avoid_touch_problems"]);
    if (omit_system_region){
        this->omit_system_region = *omit_system_region;
    }
    if (fix_touch_issue){
        this->fix_touch_issue = *fix_touch_issue;
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
    fallback_window.start();
}

CapturedWindow::~CapturedWindow(){
    release_window();
    fallback_window.stop();
}

void CapturedWindow::attach_window(HWND hwnd){
    if (!this->hwnd){
        this->hwnd = hwnd;
        hookdll_capture(hwnd, omit_system_region);
    }
}

void CapturedWindow::release_window(){
    if (hwnd){
        hookdll_uncapture(hwnd);
        hwnd = nullptr;
    }
}

bool CapturedWindow::change_window_pos(const IntRect& rect, HWND hwnd_insert_after, bool show, COLORREF bgcolor){
    if (hwnd){
        hookdll_changeWindowAtrribute(hwnd, hwnd_insert_after, rect.x, rect.y, rect.width, rect.height, show);
    }else{
        if (show){
            fallback_window.show(bgcolor, rect);
        }else{
            fallback_window.hide();
        }
    }
    return true;
}
