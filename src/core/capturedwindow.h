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
    void attach_window(HWND hwnd);
    void release_window();
    bool change_window_pos(IntRect& rect, HWND hwnd_insert_after, bool show);
};