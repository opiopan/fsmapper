//
// capturedwindow.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>
#include <string>
#include <sol/sol.hpp>
#include "viewport.h"

class MapperEngine;

class CapturedWindow{
protected:
    MapperEngine& engine;
    std::string name;
    uint32_t cwid = 0;
    HWND hwnd = nullptr;
    bool omit_system_region = true;
    bool fix_touch_issue = true;
    ViewPort::BackgroundWindow fallback_window;
    View* owner = nullptr;

public:
    CapturedWindow() = delete;
    CapturedWindow(const CapturedWindow&) = delete;
    CapturedWindow(CapturedWindow&&) = delete;
    CapturedWindow& operator = (const CapturedWindow&) = delete;
    CapturedWindow& operator = (CapturedWindow&&) = delete;

    CapturedWindow(MapperEngine& engine, uint32_t cwid, sol::object& def);
    ~CapturedWindow();

    const std::string& get_name() const {return name;};
    uint32_t get_cwid() const {return cwid;};
    HWND get_hwnd() const {return hwnd;};
    View* get_owner() const {return owner;}
    void set_owner(View* new_owner) {owner = new_owner;}
    bool need_to_fix_touch_issue()const {return fix_touch_issue;}
    void attach_window(HWND hwnd);
    void release_window();
    bool change_window_pos(const IntRect& rect, HWND hwnd_insert_after, bool show, COLORREF bgcolor = 0);
};