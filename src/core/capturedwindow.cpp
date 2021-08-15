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
    if (name || name->size() == 0){
        this->name = std::move(*name);
    }else{
        throw MapperException("varid name is not specified for the captured window");
    }
    auto omit_system_region = lua_safevalue<bool>(def["obit_system_region"]);
    if (omit_system_region){
        this->omit_system_region = *omit_system_region;
    }
}

CapturedWindow::~CapturedWindow(){
    release_window();
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

bool CapturedWindow::change_window_pos(IntRect& rect, HWND hwnd_insert_after, bool show){
    if (hwnd){
        hookdll_changeWindowAtrribute(hwnd, hwnd_insert_after, rect.x, rect.y, rect.width, rect.height, show);
    }
    return true;
}
