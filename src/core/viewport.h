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
#include <d2d1helper.h>
#include "mappercore_inner.h"
#include "tools.h"
#include "simplewindow.h"
#include "action.h"
#include "graphics.h"
#include "viewobject.h"
#include "mouseemu.h"
#include "composition.h"
#include "windowcapture.h"

class MapperEngine;
class ViewPortManager;
class CapturedWindow;
class ViewObject;

//============================================================================================
// Utilitiy classes / functions
//============================================================================================
namespace view_utils{
    struct region_def{
        bool is_relative_coordinates = true;
        FloatRect rect;
        
        region_def() = default;
        region_def(const region_def& src){*this = src;}
        region_def(const sol::table& def, bool initial_is_relative, bool default_is_whole = true, const char* msg_for_no_rect = nullptr);
        region_def& operator = (const region_def& src){
            is_relative_coordinates = src.is_relative_coordinates;
            rect = src.rect;
            return *this;
        }
    };

    struct region_restriction{
        float aspect_ratio;
        struct size{
            float width;
            float height;
        };
        std::optional<size> logical_size;

        region_restriction() = default;
        region_restriction(float aspect_ratio) : aspect_ratio(aspect_ratio){}
        region_restriction(const region_restriction& src){*this = src;}
        region_restriction& operator = (const region_restriction& src){
            aspect_ratio = src.aspect_ratio;
            logical_size = src.logical_size;
            return *this;
        }
        bool operator == (const region_restriction& lvalue) const{
            bool rc =  aspect_ratio == lvalue.aspect_ratio && 
                       logical_size.has_value() == lvalue.logical_size.has_value();
            if (logical_size && lvalue.logical_size){
                rc = rc &&
                     logical_size->width == lvalue.logical_size->width &&
                     logical_size->height == lvalue.logical_size->height;
            }
            return rc;
        }
    };
    std::optional<region_restriction> parse_region_restriction(
        const sol::table& def, const std::optional<region_restriction>& parent = std::nullopt);
    
    enum class horizontal_alignment{center, left, right};
    enum class vertical_alignment{center, top, bottom};
    struct alignment_opt{
        horizontal_alignment h = horizontal_alignment::center;
        vertical_alignment v = vertical_alignment::center;

        alignment_opt() = default;
        alignment_opt(const alignment_opt& src){*this = src;}
        alignment_opt(const sol::table& def);
        alignment_opt& operator = (const alignment_opt& src){
            h = src.h;
            v = src.v;
            return *this;
        }
    };

    FloatRect calculate_actual_rect(const FloatRect& base, const region_def& def, float scale_factor = 1.f);
    FloatRect calculate_restricted_rect(const FloatRect& base, const region_restriction& restriction, const alignment_opt& align);
    float calculate_scale_factor(const FloatRect& actual, const region_restriction& restriction, float base_factor = 1.f);
}

class ViewPort;

//============================================================================================
// View Element
//============================================================================================
template <typename Object>
class ViewElement{
public:
    using object_type = Object;
protected:
    view_utils::region_def def_region;
    view_utils::alignment_opt def_alignment;
    std::shared_ptr<object_type> object;
public:
    FloatRect region;
    FloatRect object_region;
    float object_scale_factor = 1.f;
public:
    ViewElement(const view_utils::region_def&region, const view_utils::alignment_opt& alignment, std::shared_ptr<object_type> object) :
        def_region(region), def_alignment(alignment), object(object) {};
    ~ViewElement(){};
    void calculate_element_region(const FloatRect& base, float base_scale_factor){
        if (def_region.is_relative_coordinates){
            region.x = base.x + def_region.rect.x * base.width;
            region.y = base.y + def_region.rect.y * base.height;
            region.width = def_region.rect.width * base.width;
            region.height = def_region.rect.height * base.height;
        }else{
            region.x = def_region.rect.x * base_scale_factor + base.x;
            region.y = def_region.rect.y * base_scale_factor + base.y;
            region.width = def_region.rect.width * base_scale_factor;
            region.height = def_region.rect.height * base_scale_factor;
        }
    };
    object_type& get_object() const {return *object;}
    operator object_type& () const {return *object;}
    operator const view_utils::region_def& () const {return def_region;}
    operator const view_utils::alignment_opt& () const {return def_alignment;}
};

//============================================================================================
// View
//============================================================================================
namespace capture {class captured_image;}

class View{
public:
    struct CapturedWindowAttributes{
        HWND hwnd{nullptr};
        bool need_to_avoid_touch_probrems{true};
    };
protected:
    using CWViewElement = ViewElement<CapturedWindow>;
    using NormalViewElement = ViewElement<ViewObject>;
    using CIViewElement = ViewElement<capture::captured_image>;

    ViewPort& viewport;
    std::string name;
    std::optional<view_utils::region_restriction> def_restriction;
    view_utils::alignment_opt def_alignment;
    graphics::color bg_color{0.f, 0.f, 0.f, 0.f};
    std::shared_ptr<graphics::bitmap> bg_bitmap = nullptr;
    FloatRect region;
    float scale_factor;
    std::vector<std::unique_ptr<CWViewElement>> captured_window_elements;
    std::vector<std::shared_ptr<CIViewElement>> captured_image_elements;
    std::vector<std::unique_ptr<NormalViewElement>> normal_elements;
    std::unique_ptr<EventActionMap> mappings;
    NormalViewElement* touch_captured_element = nullptr;

public:
    friend ViewPortManager;
    View() = delete;
    View(const View&) = delete;
    View(View&&) = delete;
    View& operator = (const View&) = delete;
    View& operator = (View&&) = delete;
    View(ViewPort& viewport, const char* name);
    View(MapperEngine& engine, ViewPort& viewport, sol::object& def_obj);
    ~View();
    void prepare();
    void show();
    void hide();
    void process_touch_event(ViewObject::touch_event event, int x, int y);
    void update_view(bool entire);
    bool render_view(graphics::render_target& render_target, const FloatRect& rect);
    HWND getBottomWnd();
    Action* findAction(uint64_t evid);
    int getMappingsNum(){return mappings.get() ? mappings->size() : 0;}
    bool findCapturedWindow(FloatPoint point, CapturedWindowAttributes& attrs);
    size_t getCapturedImageNum(){return captured_image_elements.size();}
};

//============================================================================================
// Viewport
//============================================================================================
class ViewPort{
protected:
public:
    static constexpr auto bg_window_class_name = "mapper_viewport_bg_window";
    class BackgroundWindow: public SimpleWindow<bg_window_class_name>{
    protected:
        using parent_class = SimpleWindow<bg_window_class_name>;
        COLORREF bgcolor{RGB(255, 255, 255)};
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

    class CoverWindow;

    using touch_event = ViewObject::touch_event;

protected:
    ViewPortManager& manager;
    std::string name;
    std::optional<int> def_display_no;
    view_utils::region_def def_region;
    std::optional<view_utils::region_restriction> def_restriction;
    view_utils::alignment_opt def_alignment;
    graphics::color bg_color {0.f, 0.f, 0.f, 1.f};
    bool is_freezed = false;
    bool is_enable = false;
    IntRect entire_region;
    IntRect entire_region_client;
    IntRect region;
    IntRect region_client;
    IntPoint window_pos;
    float scale_factor;
    std::vector<std::unique_ptr<View>> views;
    int current_view = 0;
    std::unique_ptr<graphics::render_target> render_target;
    std::unique_ptr<composition::viewport_target> composition_target;
    std::unique_ptr<CoverWindow> cover_window;
    std::unique_ptr<EventActionMap> mappings;
    int mappings_num_for_views{0};
    static constexpr auto touch_event_buffer_size = 4;
    struct {
        touch_event event;
        float x;
        float y;
    } touch_event_buffer[touch_event_buffer_size];
    int touch_event_num = 0;
    bool is_touch_captured = false;
    enum class touch_device{unknown, touch, mouse};
    touch_device captured_device = touch_device::unknown;
    DWORD touch_id = 0;;
    bool ignore_transparent_touches{false};

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
    void process_touch_event();
    void update();
    Action* findAction(uint64_t evid);
    std::pair<int, int> getMappingsStat();
    bool findCapturedWindow(FloatPoint point, View::CapturedWindowAttributes& attrs);

    // functions for views
    const std::optional<view_utils::region_restriction>& get_region_restriction()const {return def_restriction;}
    const IntRect& get_output_region() const {return region_client;}
    const IntPoint& get_window_position()const{return window_pos;}
    float get_scale_factor() const {return scale_factor;}
    const graphics::color& get_background_clolor() const {return bg_color;}
    composition::viewport_target* get_composition_target(){return composition_target.get();}
    void invalidate_rect(const FloatRect& rect);

protected:
    void clear_render_target();
};

//============================================================================================
// Viewport Manager
//============================================================================================
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
    std::unordered_map<uint32_t, std::shared_ptr<capture::image_streamer>> image_streamers;
    std::unique_ptr<mouse_emu::emulator> mouse_emulator;

public:
    ViewPortManager() = delete;
    ViewPortManager(MapperEngine& engine);
    ViewPortManager(const ViewPortManager&) = delete;
    ViewPortManager(ViewPortManager&&) = delete;
    ~ViewPortManager();
    ViewPortManager& operator =(const ViewPortManager&) = delete;
    ViewPortManager& operator =(ViewPortManager&&) = delete;

    MapperEngine& get_engine() {return engine;};
    mouse_emu::emulator& get_mouse_emulator() {return *mouse_emulator.get();}
    void init_scripting_env(sol::table& mapper_table);
    Action* find_action(uint64_t evid);
    bool findCapturedWindow(FloatPoint point, View::CapturedWindowAttributes& attrs);

    Status get_status(){
        std::lock_guard lock{mutex};
        return status;
    }

    void process_touch_event();
    void update_viewports();

    // functions to export as Lua function in mapper table
    std::shared_ptr<ViewPort> create_viewport(sol::object def_obj);
    void start_viewports();
    void stop_viewports();
    void reset_viewports();
    std::shared_ptr<CapturedWindow> create_captured_window(sol::object def_obj);

    // functions called from host program via mappercore API
    using cw_info_list = std::vector<CapturedWindowInfo>;
    cw_info_list get_captured_window_list();
    using cw_title_list = std::vector<std::string>;
    cw_title_list get_captured_window_title_list(uint32_t cwid);
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
