//
// viewport.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <sol/sol.hpp>
#include "mappercore_inner.h"
#include "tools.h"
#include "simplewindow.h"
#include "action.h"

class MapperEngine;
class ViewPortManager;
class CapturedWindow;
class ViewObject;

class ViewPort{
protected:
    class View{
    protected:
        template <typename Object>
        class ViewElement{
        public:
            using object_type = Object;
        protected:
            FloatRect def_region;
            bool is_relative_coordinates = true;
            object_type& object;
        public:
            ViewElement(FloatRect& region, bool is_relative_coordinates, object_type& object) :
                def_region(region), is_relative_coordinates(is_relative_coordinates), object(object) {};
            ~ViewElement(){};
            void transform_to_output_region(const IntRect& base, FloatRect& out) const{
                if (is_relative_coordinates){
                    out.x = base.x + def_region.x * base.width;
                    out.y = base.y + def_region.y * base.height;
                    out.width = def_region.width * base.width;
                    out.height = def_region.height * base.height;
                }else{
                    out.x = def_region.x + base.x;
                    out.y = def_region.y + base.y;
                    out.width = def_region.width;
                    out.height = def_region.height;
                }
            };
            object_type& get_object() const {return object;};
            operator object_type& () const {return object;};
        };
        using CWViewElement = ViewElement<CapturedWindow>;
        using NormalViewElement = ViewElement<ViewObject>;

        std::string name;
        std::vector<std::unique_ptr<CWViewElement>> captured_window_elements;
        std::vector<std::unique_ptr<NormalViewElement>> normal_elements;
        std::unique_ptr<EventActionMap> mappings;

    public:
        friend ViewPortManager;
        View() = delete;
        View(const View&) = delete;
        View(View&&) = delete;
        View& operator = (const View&) = delete;
        View& operator = (View&&) = delete;
        View(MapperEngine& engine, sol::object& def_obj);
        ~View();
        void show(ViewPort& viewport);
        void hide(ViewPort& viewport);
        HWND getBottomWnd();
        Action* findAction(uint64_t evid);
        int getMappingsNum(){return mappings.get() ? mappings->size() : 0;}
    };

public:
    static constexpr auto bg_window_class_name = "mapper_viewport_bg_window";
    class BackgroundWindow: public SimpleWindow<bg_window_class_name>{
    protected:
        using parent_class = SimpleWindow<bg_window_class_name>;
        COLORREF bgcolor;
        GdiObject<HBRUSH> bgbrush;
    public:
        BackgroundWindow(const WinDispatcher& dispatcher = WinDispatcher::sharedDispatcher()):SimpleWindow(dispatcher){};
        virtual ~BackgroundWindow() = default;
        void start(){
            create();
        }
        void start(COLORREF bgcolor, const IntRect& rect, HWND hWndInsertAfter = nullptr){
            this->bgcolor = bgcolor;
            bgbrush = CreateSolidBrush(bgcolor);
            create();
            ::SetWindowPos(*this, hWndInsertAfter ? hWndInsertAfter : HWND_TOP, rect.x, rect.y, rect.width, rect.height, 0);
        };
        void stop(){destroy();};
        void show(){showWindow(SW_SHOW);}
        void show(COLORREF bgcolor, const IntRect& rect, HWND hWndInsertAfter = nullptr){
            if (this->bgcolor != bgcolor){
                this->bgcolor = bgcolor;
                bgbrush = CreateSolidBrush(bgcolor);
            }
            ::SetWindowPos(*this, hWndInsertAfter ? hWndInsertAfter : HWND_TOP, rect.x, rect.y, rect.width, rect.height, 0);
            showWindow(SW_SHOW);
        }
        void hide(){showWindow(SW_HIDE);}
    protected:
        bool onEraseBackground(HDC hdc) override{
            RECT rect;
            ::GetClientRect(*this, &rect);
            ::FillRect(hdc, &rect, bgbrush);
            return true;
        }
    };

protected:
    ViewPortManager& manager;
    std::string name;
    std::optional<int> def_display_no;
    FloatRect def_region = {0., 0., 1., 1.};
    COLORREF bg_color = 0x000000;
    bool is_relative_coordinates = true;
    bool is_freezed = false;
    bool is_enable = false;
    IntRect region;
    std::vector<std::unique_ptr<View>> views;
    int current_view = 0;
    BackgroundWindow bgwin;
    std::unique_ptr<EventActionMap> mappings;
    int mappings_num_for_views{0};

public:
    friend ViewPortManager;
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
    void setMappings(sol::object mapdef);
    void addMappings(sol::object mapdef);

    // functions to export to ViewPortManager
    void freeze(){is_freezed = true;};
    void enable(const std::vector<IntRect> displays);
    void disable();
    Action* findAction(uint64_t evid);
    std::pair<int, int> getMappingsStat();

    // functions for views
    const IntRect& get_output_region() const {return region;};
    COLORREF get_background_clolor() const {return bg_color;}
};

class MapperEngine;

class ViewPortManager{
public:
    enum class Status{
        init,
        ready_to_start,
        starting,
        running,
        suspending,
        suspended,
    };
protected:
    std::mutex mutex;
    std::condition_variable cv;
    MapperEngine& engine;
    Status status = Status::init;
    std::vector<std::shared_ptr<ViewPort>> viewports;
    std::vector<IntRect> displays;
    uint32_t cwid_counter = 1;
    std::unordered_map<uint32_t, std::shared_ptr<CapturedWindow>> captured_windows;

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
    Action* find_action(uint64_t evid);

    Status get_status(){
        std::lock_guard lock{mutex};
        return status;
    }

    // functions to export as Lua function in mapper table
    std::shared_ptr<ViewPort> create_viewport(sol::object def_obj);
    void start_viewports();
    void stop_viewports();
    void reset_viewports();
    std::shared_ptr<CapturedWindow> create_captured_window(sol::object def_obj);

    // functions called from host program via mappercore API
    using cw_info_list = std::vector<CapturedWindowInfo>;
    cw_info_list get_captured_window_list();
    void register_captured_window(uint32_t cwid, HWND hWnd);
    void unregister_captured_window(uint32_t cwid);
    using vp_info_list = std::vector<ViewportInfo>;
    vp_info_list get_viewport_list();
    void enable_viewports();
    void disable_viewports();
    std::pair<int, int> get_mappings_stat();

protected:
    void change_status(Status status){
        if (this->status == Status::init && status != Status::init){
            for (auto& viewport : viewports){
                viewport->freeze();
            }
        }
        this->status = status;
        cv.notify_all();
    };
    void enable_viewport_primitive();
    void disable_viewport_primitive();
    static BOOL monitor_enum_proc(HMONITOR hmon, HDC hdc, LPRECT rect, LPARAM context);
    static void notify_close_proc(HWND hWnd, void* context);
    void process_close_event(HWND hWnd);
};
