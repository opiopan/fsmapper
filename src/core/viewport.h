//
// viewport.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <sol/sol.hpp>
#include "tools.h"
#include "simplewindow.h"

template <typename T> 
struct RectangleBase{
    T x = 0;
    T y = 0;
    T width = 0;
    T height = 0;

    RectangleBase() = default;
    RectangleBase(T x, T y, T width, T height) : x(x), y(y), width(width), height(height){};
    RectangleBase(const RectangleBase& src){*this = src;};
    ~RectangleBase() = default;

    RectangleBase& operator = (const RectangleBase& src){
        x = src.x;
        y = src.y;
        width = src.width;
        height = src.height;
        return *this;
    };
};

using IntRect = RectangleBase<int>;
using FloatRect = RectangleBase<float>;

class PresentationObject{
};

class ViewPortManager;

class ViewPort{
protected:
    class View{
    };

    static constexpr WindowClassName bg_window_class_name = {"mapper_viewport_bg_window"};
    class BackgroundWindow: public SimpleWindow<bg_window_class_name, WS_POPUP>{
    protected:
        using parent_class = SimpleWindow<bg_window_class_name, WS_POPUP>;
        COLORREF bgcolor;
    public:
        BackgroundWindow() = default;
        virtual ~BackgroundWindow() = default;
        void start(COLORREF bgcolor, const IntRect& rect, HWND hWndInsertAfter = nullptr){
            this->bgcolor = bgcolor;
            create();
            ::SetWindowPos(*this, hWndInsertAfter ? hWndInsertAfter : HWND_TOP, rect.x, rect.y, rect.width, rect.height, 0);
            showWindow(SW_SHOW);
        };
        void stop(){destroy();};
    protected:
        virtual void preRegisterClass(WNDCLASSEXA& wc) override{
            parent_class::preRegisterClass(wc);
            wc.hbrBackground = CreateSolidBrush(bgcolor);
        };
    };

protected:
    ViewPortManager& manager;
    std::string name;
    std::optional<int> def_display_no;
    FloatRect def_region = {0., 0., 1., 1.};
    COLORREF bg_color = 0x000000;
    bool is_relative_coordinates = true;
    IntRect region;
    std::vector<View> views;
    int current_view = 0;
    BackgroundWindow bgwin;

public:
    ViewPort() = delete;
    ViewPort(const ViewPort&) = delete;
    ViewPort(ViewPort&&) = delete;
    ViewPort(ViewPortManager& manager, sol::object def_obj);
    ~ViewPort();
    ViewPort& operator = (const ViewPort&) = delete;
    ViewPort& operator = (ViewPort&&) = delete;

    // functions to export as Lua table member
    int registerView(sol::object def_obj);
    std::optional<int> getCurrentView();
    void setCurrentView(sol::optional<int> view_no_obj);

    // functions to export to ViewPortManager
    void enable(const std::vector<IntRect> displays);
    void disable();
};

class MapperEngine;

class ViewPortManager{
protected:
    enum class Status{
        init,
        ready_to_start,
        starting,
        running,
        suspending,
        suspended,
    };
    std::mutex mutex;
    std::condition_variable cv;
    MapperEngine& engine;
    Status status = Status::init;
    std::vector<std::shared_ptr<ViewPort>> viewports;
    std::vector<IntRect> displays;
    uint32_t cwid_counter = 1;

public:
    ViewPortManager() = delete;
    ViewPortManager(MapperEngine& engine);
    ViewPortManager(const ViewPortManager&) = delete;
    ViewPortManager(ViewPortManager&&) = delete;
    ~ViewPortManager();
    ViewPortManager& operator =(const ViewPortManager&) = delete;
    ViewPortManager& operator =(ViewPortManager&&) = delete;

    MapperEngine& get_engine() {return engine;};

    void init_scripting_env(sol::table& mapper_table);

    // functions to export as Lua function in mapper table
    std::shared_ptr<ViewPort> create_viewvort(sol::object def_obj);
    void start_viewports();
    void stop_viewports();
    void reset_viewports();

    // functions called from host program via mappercore API
    void register_captured_window(uint32_t cwid, HWND hWnd);
    void unregister_captured_window(uint32_t cwid);
    void enable_viewports();
    void disable_viewports();

protected:
    void change_status(Status status){
        this->status = status;
        cv.notify_all();
    };
    void enable_viewport_primitive();
    void disable_viewport_primitive();
    static BOOL monitor_enum_proc(HMONITOR hmon, HDC hdc, LPRECT rect, LPARAM context);
};
