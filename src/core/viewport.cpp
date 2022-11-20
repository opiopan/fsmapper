//
// viewport.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <stdexcept>
#include <cfenv>
#include <cmath>
#include <chrono>
#include <functional>
#include "viewport.h"
#include "engine.h"
#include "capturedwindow.h"
#include "viewobject.h"
#include "graphics.h"
#include "tools.h"
#include "hookdll.h"

#include <windowsx.h>
#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>

static constexpr auto MOUSE_REJECTION_TIME = std::chrono::milliseconds(500);

static ViewPortManager* the_manager = nullptr;

//============================================================================================
// Utilities
//============================================================================================
namespace view_utils{
    region_def::region_def(const sol::table& def, bool initial_is_relative, bool default_is_whole, const char* msg_for_no_rect){
        is_relative_coordinates = initial_is_relative;
        auto&& coordinate = lua_safevalue<std::string>(def["coordinate"]);
        if (coordinate && coordinate.value() == "relative"){
            is_relative_coordinates = true;
        }else if (coordinate && coordinate.value() == "absolute"){
            is_relative_coordinates = false;
        }else if (coordinate){
            throw MapperException("Invalid value for \"coordinate\" parameter, \"relative\" or \"absolute\" can be specified");
        }
        auto x = lua_safevalue<float>(def["x"]);
        auto y = lua_safevalue<float>(def["y"]);
        auto width = lua_safevalue<float>(def["width"]);
        auto height = lua_safevalue<float>(def["height"]);
        if (x && y && width && height){
            rect = FloatRect{*x, *y, *width, *height};
        }else if (default_is_whole){
            is_relative_coordinates = true;
            rect = FloatRect{0.f, 0.f, 1.f, 1.f};
        }else{
            throw MapperException(msg_for_no_rect);
        }
    }

    std::optional<region_restriction> parse_region_restriction(
        const sol::table& def, const std::optional<region_restriction>& parent){
        auto width = lua_safevalue<float>(def["logical_width"]);
        auto height = lua_safevalue<float>(def["logical_height"]);
        auto aspect_ratio = lua_safevalue<float>(def["aspect_ratio"]);
        if (width && height){
            region_restriction restriction;
            restriction.logical_size = region_restriction::size{*width, *height};
            restriction.aspect_ratio = *width / *height;
            return restriction;
        }else if (aspect_ratio){
            region_restriction restriction;
            restriction.aspect_ratio = *aspect_ratio;
            return restriction;
        }else if (parent){
            return parent;
        }else{
            return std::nullopt;
        }
    }

    alignment_opt::alignment_opt(const sol::table& def){
        auto&& h_alignment = lua_safevalue<std::string>(def["horizontal_alignment"]);
        if (h_alignment){
            if (*h_alignment == "center"){
                h = horizontal_alignment::center;
            }else if (*h_alignment == "left"){
                h = horizontal_alignment::left;
            }else if (*h_alignment == "right"){
                h = horizontal_alignment::right;
            }else{
                throw MapperException(
                    "value of \"horizontal_alignment\" parameter must be eather \"center\", \"left\", or \"right\"");
            }
        }else{
            h = horizontal_alignment::center;
        }
        auto&& v_alignment = lua_safevalue<std::string>(def["vertical_alignment"]);
        if (v_alignment){
            if (*v_alignment == "center"){
                v = vertical_alignment::center;
            }else if (*v_alignment == "top"){
                v = vertical_alignment::top;
            }else if (*v_alignment == "bottom"){
                v = vertical_alignment::bottom;
            }else{
                throw MapperException(
                    "value of \"vertical_alignment\" parameter must be eather \"center\", \"top\", or \"bottom\"");
            }
        }else{
            v = vertical_alignment::center;
        }
    }

    FloatRect calculate_actual_rect(const FloatRect& base, const region_def& def, float scale_factor){
        if (def.is_relative_coordinates){
            return FloatRect{
                base.x + def.rect.x * base.width,
                base.y + def.rect.y * base.height,
                def.rect.width * base.width,
                def.rect.height * base.height
            };
        }else{
            return FloatRect{
                base.x + def.rect.x * scale_factor,
                base.y + def.rect.y * scale_factor,
                def.rect.width * scale_factor,
                def.rect.height * scale_factor
            };
        }
    }

    FloatRect calculate_restricted_rect(const FloatRect& base, const region_restriction& restriction, const alignment_opt& align){
        auto aratio = base.width / base.height;
        if (aratio > restriction.aspect_ratio){
            auto factor = restriction.aspect_ratio / aratio;
            auto width = base.width * factor;
            auto offset = 
                align.h == horizontal_alignment::center ? (base.width - width) / 2.f :
                align.h == horizontal_alignment::right  ? (base.width - width) :
                                                          0.f;
            return {
                base.x + offset,
                base.y,
                width,
                base.height
            };
        }else{
            auto factor = aratio / restriction.aspect_ratio;
            auto height = base.height * factor;
            auto offset = 
                align.v == vertical_alignment::center ? (base.height - height) / 2.f :
                align.v == vertical_alignment::bottom ? (base.height - height) :
                                                        0.f;
            return {
                base.x,
                base.y + offset,
                base.width,
                height
            };
        }
    }

    float calculate_scale_factor(const FloatRect& actual, const region_restriction& restriction, float base_factor){
        if (restriction.logical_size){
            return actual.width / restriction.logical_size->width;
        }else{
            return base_factor;
        }
    }
}

//============================================================================================
// Low level mouse hook
//    This funciton avoids several problems to use pop out windows of FS2020 with touch display.
//    The all mouse messages generated touch action on the captured windows will be droped by
//    this hook procedure. On the other hand, new other mouse messages wich can be handled by
//    FS2020 correctly will be generated on the other thread instead of the original mouse 
//    messages.
//============================================================================================
static HHOOK hookHandle = 0;

static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam){
    static constexpr auto touch_mask = 0xFFFFFF00;
    static constexpr auto touch_signature = 0xFF515700;
    static constexpr auto delay_down = mouse_emu::milliseconds(50);
    static constexpr auto delay_up = mouse_emu::milliseconds(50);
    static auto in_down_state = true;
    static POINT last_down_point;
    static mouse_emu::clock::time_point last_ops_time;

    if (nCode >= HC_ACTION) {
        auto mouse = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        View::CapturedWindowAttributes captured_window;
        if (the_manager->findCapturedWindow({IntPoint{mouse->pt.x, mouse->pt.y}}, captured_window)){
            if ((mouse->dwExtraInfo & touch_mask) == touch_signature && captured_window.need_to_avoid_touch_probrems){
                auto now = mouse_emu::clock::now();
                if (wParam == WM_LBUTTONDOWN){
                    if (in_down_state){
                        auto target_time = last_ops_time + delay_up;
                        the_manager->get_mouse_emulator().emulate(mouse_emu::event::up, last_down_point.x, last_down_point.y, target_time);
                        now = max(target_time, now);
                    }
                    the_manager->get_mouse_emulator().emulate(mouse_emu::event::move, mouse->pt.x, mouse->pt.y, now);
                    in_down_state = true;
                    last_down_point = mouse->pt;
                    last_ops_time = max(last_ops_time + delay_down, now + delay_down);
                    the_manager->get_mouse_emulator().emulate(mouse_emu::event::down, mouse->pt.x, mouse->pt.y, last_ops_time);
                }else if (wParam == WM_LBUTTONUP){
                    in_down_state = false;
                    last_ops_time = max(now + delay_up, last_ops_time + delay_up);
                    the_manager->get_mouse_emulator().emulate(mouse_emu::event::up, mouse->pt.x, mouse->pt.y, last_ops_time);
                }else if (wParam == WM_MOUSEMOVE){
                    last_ops_time = max(last_ops_time, now);
                    the_manager->get_mouse_emulator().emulate(mouse_emu::event::move, mouse->pt.x, mouse->pt.y, last_ops_time);
                }
                return 1;
            }
        }
    }
    return CallNextHookEx(hookHandle, nCode, wParam, lParam);
}

static void install_mouse_hook(){
    if (hookHandle){
        abort();
    }
    WinDispatcher::sharedDispatcher().invoke([]{
        HMODULE hmodule = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<const char*>(mouseHookProc), &hmodule);
        hookHandle = SetWindowsHookExW(WH_MOUSE_LL, mouseHookProc, hmodule, 0);
    });
}

static void uninstall_mouse_hook(){
    if (hookHandle){
        WinDispatcher::sharedDispatcher().invoke([]{
            UnhookWindowsHookEx(hookHandle);
            hookHandle = nullptr;
        });
    }
}

//============================================================================================
// View implementation
//============================================================================================
View::View(ViewPort& viewport, const char* name):viewport(viewport), name(name){
    def_restriction = viewport.get_region_restriction();
}

View::View(MapperEngine& engine, ViewPort& viewport, sol::object& def_obj) : viewport(viewport){
    if (def_obj.get_type() != sol::type::table){
        throw MapperException("view definition must be specified as a table");
    }
    auto def = def_obj.as<sol::table>();

    name = std::move(lua_safestring(def["name"]));
    if (name.length() == 0){
        throw MapperException("\"name\" parameter for view definition must be specified");
    }

    def_restriction = view_utils::parse_region_restriction(def, viewport.get_region_restriction());
    def_alignment = view_utils::alignment_opt(def);
    sol::object background = def["background"];
    if (background.is<graphics::bitmap&>()){
        bg_bitmap = background.as<std::shared_ptr<graphics::bitmap>>();
    }else if (background.get_type() != sol::type::lua_nil){
        bg_color = graphics::color(background);
    }

    sol::object elements_val = def["elements"];
    if (elements_val.get_type() == sol::type::table){
        sol::table elements_def = elements_val;
        for (int i = 1; i <= elements_def.size(); i++){
            sol::object item_obj = elements_def[i];
            if (item_obj.is<sol::table>()){
                auto item = item_obj.as<sol::table>();
                auto region_def = view_utils::region_def(item, !(def_restriction && def_restriction->logical_size));
                view_utils::alignment_opt alignment(item);
                sol::object object = item["object"];
                if (object.is<CapturedWindow&>()){
                    auto element = std::make_unique<CWViewElement>(region_def, alignment, object.as<std::shared_ptr<CapturedWindow>>());
                    captured_window_elements.push_back(std::move(element));
                }else{
                    auto view_object = as_view_object(object);
                    if (view_object){
                        auto element = std::make_unique<NormalViewElement>(region_def, alignment, view_object);
                        normal_elements.push_back(std::move(element));
                    }else{
                        throw MapperException("unsupported object is specified as view element object");
                    }
                }
            }
        }
    }
    if (captured_window_elements.size() == 0 && normal_elements.size() == 0){
        throw MapperException("there is no view element difinition, at least one view element is required");
    }

    mappings = std::move(createEventActionMap(engine, def["mappings"]));
}

View::~View(){
}

void View::prepare(){
    if (def_restriction == viewport.get_region_restriction()){
        region = viewport.get_output_region();
        scale_factor = viewport.get_scale_factor();
    }else{
        region = view_utils::calculate_restricted_rect(viewport.get_output_region(), *def_restriction, def_alignment);
        scale_factor = view_utils::calculate_scale_factor(region, *def_restriction, viewport.get_scale_factor());
    }
    for (auto& element : captured_window_elements){
        element->calculate_element_region(region + viewport.get_window_position(), scale_factor);
        element->object_region = element->region;
        element->object_scale_factor = scale_factor;
    }
    for (auto& element : normal_elements){
        element->calculate_element_region(region, scale_factor);
        auto aratio = element->get_object().get_aspect_ratio();
        view_utils::region_restriction restriction{aratio ? *aratio : element->region.width / element->region.height};
        element->object_region = view_utils::calculate_restricted_rect(element->region, restriction, *element);
        element->object_scale_factor = element->get_object().calculate_scale_factor(element->object_region, scale_factor);
    }
}

void View::show(){
    std::for_each(std::rbegin(captured_window_elements), std::rend(captured_window_elements), [this](auto& element){
        element->get_object().set_owner(this);
        element->get_object().change_window_pos(IntRect{element->region}, HWND_TOP, true, viewport.get_background_clolor());
    });
    FloatRect rect{viewport.get_output_region()};
    viewport.invaridate_rect(rect);
}

void View::hide(){
    for (auto& element : captured_window_elements){
        auto owner = element->get_object().get_owner();
        if (owner == this){
            element->get_object().set_owner(nullptr);
            element->get_object().change_window_pos(IntRect{element->region}, HWND_BOTTOM, false);
        }
    }
    for (auto& element : normal_elements){
        element->get_object().reset_touch_status();
    }
    touch_captured_element = nullptr;
}

void View::process_touch_event(ViewObject::touch_event event, int x, int y){
    if (touch_captured_element){
        auto& object_region = touch_captured_element->object_region;
        auto& object  = touch_captured_element->get_object();
        auto rel_x = (static_cast<float>(x) - object_region.x) / object_region.width;
        auto rel_y = (static_cast<float>(y) - object_region.y) / object_region.height;
        auto result = object.process_touch_event(event, rel_x, rel_y, object_region);
        if (result != ViewObject::touch_reaction::capture){
            touch_captured_element = nullptr;
        }
    }else if (event == ViewObject::touch_event::down){
        for (auto& element : normal_elements){
            if (element->object_region.pointIsInRectangle(x, y)){
                auto& object_region = element->object_region;
                auto rel_x = (static_cast<float>(x) - object_region.x) / object_region.width;
                auto rel_y = (static_cast<float>(y) - object_region.y) / object_region.height;
                auto result = element->get_object().process_touch_event(event, rel_x, rel_y, object_region);
                if (result == ViewObject::touch_reaction::capture){
                    touch_captured_element = element.get();
                }
                break;
            }
        }
    }
}

void View::update_view(){
    FloatRect dirty_rect;
    std::for_each(std::rbegin(normal_elements), std::rend(normal_elements), [&](auto& element){
        element->get_object().merge_dirty_rect(element->object_region, dirty_rect);
    });
    if (dirty_rect.width > 0.f && dirty_rect.height > 0.f){
        dirty_rect.x -= 1.f;
        dirty_rect.width += 2.f;
        dirty_rect.y -=1.f;
        dirty_rect.height += 2.f;
        viewport.invaridate_rect(dirty_rect);
    }
}

bool View::render_view(graphics::render_target& render_target, const FloatRect& rect){
    //fill outer area of varid region as needed, then clear background
    auto clear_background = [this, &render_target]{
        render_target->Clear(bg_color);
        if (bg_bitmap){
            bg_bitmap->draw(render_target, region);
        }
    };
    if (rect.width > region.width || rect.height > region.height){
        render_target->Clear(viewport.get_background_clolor());
        render_target->PushAxisAlignedClip(region, D2D1_ANTIALIAS_MODE_ALIASED);
        clear_background();
        render_target->PopAxisAlignedClip();
    }else{
        render_target->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_ALIASED);
        clear_background();
        render_target->PopAxisAlignedClip();
    }

    // render each objects are proceded below
    std::for_each(std::rbegin(normal_elements), std::rend(normal_elements), [&](auto& element){
        if (element->object_region.isIntersected(rect)){
            element->get_object().update_rect(render_target, element->object_region, element->object_scale_factor);
        }
    });

    return true;
}

HWND View::getBottomWnd(){
    if (captured_window_elements.size() > 0){
        return captured_window_elements[captured_window_elements.size() -1 ]->get_object().get_hwnd();
    }else{
        return nullptr;
    }
}

Action* View::findAction(uint64_t evid){
    if (mappings && mappings->count(evid)){
        return mappings->at(evid).get();
    }else{
        return nullptr;
    }
}

bool View::findCapturedWindow(FloatPoint point, CapturedWindowAttributes& attrs){
    for (auto& element : captured_window_elements){
        if (element->region.pointIsInRectangle(point.x, point.y)){
            attrs.hwnd = element->get_object().get_hwnd();
            attrs.need_to_avoid_touch_probrems =element->get_object().need_to_fix_touch_issue();
            return true;
        }
    }
    return false;
}

//============================================================================================
// Viewport Cover Window inmplementation
//============================================================================================
static constexpr auto cover_window_name = "mapper_viewport_cover_window";
class ViewPort::CoverWindow: public SimpleWindow<cover_window_name>{
public:
    CoverWindow() = default;
    virtual ~CoverWindow() = default;
    virtual void start(const IntRect& vrect, const IntRect& erect) = 0;
    virtual void stop() = 0;
    virtual void update_window() = 0;
    virtual void update_window_with_dc(HDC hdc) = 0;
};

template <typename MSG_RELAY, typename UPDATE_WINDOW>
class ViewPortWindow : public ViewPort::CoverWindow{
protected:
    using base_class = ViewPort::CoverWindow;
    UINT update_msg;
    MSG_RELAY relay;
    UPDATE_WINDOW on_update_window;
    COLORREF bgcolor;
    IntRect varid_rect;
    IntRect entire_rect;
    POINT window_pos;
    SIZE window_size;
    BLENDFUNCTION blend;
    UPDATELAYEREDWINDOWINFO lw_info{0};
    std::chrono::steady_clock::time_point last_touch;

public:
    ViewPortWindow(COLORREF bgcolor, const MSG_RELAY& relay, const UPDATE_WINDOW& on_update_window):
        bgcolor(bgcolor), relay(relay), on_update_window(on_update_window){
    	update_msg = ::RegisterWindowMessageA("MAPPER_CORE_VIEW_UPDATE");
        last_touch = std::chrono::steady_clock::now();
    }
    virtual ~ViewPortWindow() = default;

    void start(const IntRect& erect, const IntRect& vrect) override{
        entire_rect = erect;
        varid_rect = vrect;
        create();
        showWindow(SW_SHOW);
    }

    void stop() override{
        destroy();
    }

    void update_window() override{
        ::PostMessageA(*this, update_msg, 0, 0);
    }

    void update_window_with_dc(HDC dc) override{
        lw_info.hdcSrc = dc;
        ::UpdateLayeredWindowIndirect(*this, &lw_info);
    }

protected:
    LRESULT messageProc(UINT msg, WPARAM wparam, LPARAM lparam) override{
        if (msg == WM_TOUCH){
            last_touch = std::chrono::steady_clock::now();
            auto rc = relay(msg, wparam, lparam);
            ::CloseTouchInputHandle(reinterpret_cast<HTOUCHINPUT>(lparam));
            return rc;
        }else if (msg == WM_MOUSEMOVE || 
            msg == WM_LBUTTONUP || msg == WM_LBUTTONDOWN){
            auto now = std::chrono::steady_clock::now();
            if (now - last_touch > MOUSE_REJECTION_TIME){
                return relay(msg, wparam, lparam);
            }else{
                return base_class::messageProc(msg, wparam, lparam);
            }
        }else if (msg == update_msg){
            on_update_window();
        }else{
            return base_class::messageProc(msg, wparam, lparam);
        }
    }

    void preCreateWindow(CREATESTRUCTA& cs) override{
        base_class::preCreateWindow(cs);
        cs.dwExStyle = WS_EX_LAYERED | WS_EX_TOPMOST;
        cs.x = entire_rect.x;
        cs.y = entire_rect.y;
        cs.cx = entire_rect.width;
        cs.cy = entire_rect.height;
    }

    bool onCreate(CREATESTRUCT* cs) override{
        base_class::onCreate(cs);

        window_pos.x = entire_rect.x;
        window_pos.y = entire_rect.y;
        window_size.cx = entire_rect.width;
        window_size.cy = entire_rect.height;
        static POINT point{0, 0};
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;
        lw_info.cbSize = sizeof(lw_info);
        lw_info.pptDst = &window_pos;
        lw_info.pptSrc = &point;
        lw_info.psize = &window_size;
        lw_info.pblend = &blend;
        lw_info.dwFlags = ULW_ALPHA;

        ::RegisterTouchWindow(hWnd, 0);
        
        return true;
    }
};

template <typename MSG_RELAY, typename UPDATE_WINDOW>
std::unique_ptr<ViewPortWindow<MSG_RELAY, UPDATE_WINDOW>> make_viewport_window(
    COLORREF bgcolor, const MSG_RELAY& relay, const UPDATE_WINDOW& update_window){
    return std::move(std::make_unique<ViewPortWindow<MSG_RELAY, UPDATE_WINDOW>>(bgcolor, relay, update_window));
}

//============================================================================================
// Viewport inmplementation
//============================================================================================
ViewPort::ViewPort(ViewPortManager& manager, sol::object def_obj): manager(manager){
    if (def_obj.get_type() != sol::type::table){
        throw MapperException("viewport definition must be specified as a table");
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
    def_region = view_utils::region_def(
        def, static_cast<bool>(def_display_no), static_cast<bool>(def_display_no),
        "Viewport region must be specified by \"x\", \"y\", \"width\" and \"height\" parameters, "
        "if absolute coordinate system is selected or target display number specification is ommited");
    
    if (!def_display_no && def_region.is_relative_coordinates){
        throw MapperException("coordinate of viewport should be \"absolute\" if target display number specification is ommited");
    }

    def_restriction = view_utils::parse_region_restriction(def);
    def_alignment = view_utils::alignment_opt(def);

    sol::object bgcolor = def["bgcolor"];
    if (bgcolor.get_type() != sol::type::nil){
        bg_color = graphics::color(bgcolor);
        bg_color.set_alpha(1.f);
    }

    auto view = std::make_unique<View>(*this, "empty view");
    views.push_back(std::move(view));

    auto window = make_viewport_window(bg_color,
        //
        // processing mouse message and touch message
        //
        [this](UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT{
            std::lock_guard lock(touch_event_mutex);
            auto need_notify = touch_event_num == 0;
            using tevent = ViewPort::touch_event;
            using oevent = std::optional<tevent>;
            oevent event;
            float x = -1.f;
            float y = -1.f;
            DWORD msg_touch_id = 0;
            std::function<void()> start_capture = []{};
            std::function<void()> end_capture = []{};

            if (msg == WM_TOUCH){
                if (is_touch_captured && captured_device == touch_device::mouse){
                    event = tevent::cancel;
                    end_capture = [this]{
                        is_touch_captured = false;
                        captured_device = touch_device::unknown;
                        ::ReleaseCapture();
                    };
                }else{
                    auto handle = reinterpret_cast<HTOUCHINPUT>(lparam);
                    if (LOWORD(wparam) > 1){
                        event = tevent::cancel;
                    }else{
                        TOUCHINPUT  input;
                        ::GetTouchInputInfo(handle, 1, &input, sizeof(TOUCHINPUT));
                        msg_touch_id = input.dwID;
                        event = input.dwFlags & TOUCHEVENTF_DOWN ? oevent{tevent::down} :
                                input.dwFlags & TOUCHEVENTF_UP ?   oevent{tevent::up} :
                                input.dwFlags & TOUCHEVENTF_MOVE ? oevent{tevent::drag} :
                                                                   oevent{std::nullopt};
                        POINT pt{ input.x / 100, input.y / 100 };
                        ::ScreenToClient(*cover_window, &pt);
                        x = pt.x;
                        y = pt.y;
                    }
                    start_capture = [this, msg_touch_id]{
                        is_touch_captured = true;
                        captured_device = touch_device::touch;
                        touch_id = msg_touch_id;
                        ::SetCapture(*cover_window);
                    };
                    end_capture = [this]{
                        is_touch_captured = false;
                        captured_device = touch_device::unknown;
                        ::ReleaseCapture();
                    };
                }
            }else{
                if (is_touch_captured && captured_device == touch_device::touch){
                    event = tevent::cancel;
                    end_capture = [this]{
                        is_touch_captured = false;
                        captured_device = touch_device::unknown;
                    };
                }else{
                    event = msg == WM_LBUTTONDOWN ? oevent{tevent::down}  :
                            msg == WM_LBUTTONUP ?   oevent{tevent::up} :
                            msg == WM_MOUSEMOVE ?   oevent{tevent::drag} :
                                                    oevent{std::nullopt};
                    x = GET_X_LPARAM(lparam);
                    y = GET_Y_LPARAM(lparam);
                    start_capture = [this]{
                        is_touch_captured = true;
                        captured_device = touch_device::mouse;
                        ::SetCapture(*cover_window);
                    };
                    end_capture = [this]{
                        is_touch_captured = false;
                        captured_device = touch_device::unknown;
                        ::ReleaseCapture();
                    };
                }
            }
            if (event){
                if (*event == tevent::down){
                    this->manager.get_mouse_emulator().emulate(mouse_emu::event::cancel_recovery, 0, 0, mouse_emu::clock::now());
                }else if (*event == tevent::up || *event == tevent::cancel){
                    this->manager.get_mouse_emulator().emulate(mouse_emu::event::recover, 0, 0, mouse_emu::clock::now());
                }

                if (!is_touch_captured && *event != tevent::down){
                    return 0;
                }

                if (touch_event_num < touch_event_buffer_size){
                    if (*event == tevent::drag && touch_event_num > 0 && 
                        touch_event_buffer[touch_event_num - 1].event == tevent::drag){
                        auto& slot = touch_event_buffer[touch_event_num - 1];
                        slot.x = x;
                        slot.y = y;
                    }else{
                        auto& slot = touch_event_buffer[touch_event_num];
                        slot.event = *event;
                        slot.x = x;
                        slot.y = y;
                        touch_event_num++;
                    }
                    if (*event == tevent::down){
                        start_capture();
                    }else if (*event == tevent::up || *event == tevent::cancel){
                        end_capture();
                    }
                    if (need_notify){
                        mapper_EngineInstance()->notifyTouchEvent();
                    }
                }else{
                    auto& last = touch_event_buffer[touch_event_num - 1];
                    if (*event == tevent::down){
                        return 0;
                    }else if (*event == tevent::drag){
                        if (last.event == tevent::drag){
                            last.x = x;
                            last.y = y;
                        }else{
                            return 0;
                        }
                    }else if (*event == tevent::up){
                        if (last.event == tevent::drag){
                            last.event = *event;
                            last.x = x;
                            last.y = y;
                        }else if (last.event == tevent::down){
                            touch_event_num--;
                        }
                        end_capture();
                    }else if (*event == tevent::cancel){
                        last.event = *event;
                        end_capture();
                    }
                }
            }
            return 0;
        },

        //
        // updating viewport window contents displayed
        // note thta contents to display is build as bitmap in scripting thread
        // this function just reflect that on layerd window
        //
        [this](){
            std::lock_guard lock(rendering_mutex);
            if (rendering_count > rendering_reflect_count){
                rendering_reflect_count = rendering_count;
                (*render_target)->BeginDraw();
                {
                    CComPtr<ID2D1GdiInteropRenderTarget> gdi_target;
                    (*render_target)->QueryInterface(&gdi_target);
                    HDC dc;
                    gdi_target->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &dc);
                    cover_window->update_window_with_dc(dc);
                    gdi_target->ReleaseDC(nullptr);
                }
                (*render_target)->EndDraw();
            }
        }
    );
    cover_window = std::move(window);
}

ViewPort::~ViewPort(){
    disable();
}

void ViewPort::enable(const std::vector<IntRect> displays){
    if (is_enable) {
        return;
    }
    if (views.size() == 0){
        std::ostringstream os;
        os << "no view is registerd to the viewport \"" << name << "\"";
        throw MapperException(std::move(os.str()));
    }
    if (!def_display_no){
        entire_region = view_utils::calculate_actual_rect({0.f, 0.f, 0.f, 0.f}, def_region).to_IntRect();
    }else{
        if (displays.size() < *def_display_no){
            std::ostringstream os;
            os << "Display number that is specified as viewport difinition is invalid. [viewport: " << name ;
            os << "] [display: " << *def_display_no << "]";
            throw MapperException(std::move(os.str()));
        }else{
            auto& drect = displays[*def_display_no - 1];
            entire_region = view_utils::calculate_actual_rect(drect, def_region).to_IntRect();
        }
    }
    if (def_restriction){
        region = view_utils::calculate_restricted_rect(entire_region, *def_restriction, def_alignment).to_IntRect();
        scale_factor = view_utils::calculate_scale_factor(region, *def_restriction);
    }else{
        region = entire_region;
        scale_factor = 1.f;
    }
    window_pos = entire_region;
    entire_region_client = entire_region;
    entire_region_client.x = 0;
    entire_region_client.y = 0;
    region_client = region;
    region_client.x = region.x - entire_region.x;
    region_client.y = region.y - entire_region.y;
    is_enable = true;
    render_target = std::move(graphics::render_target::create_render_target(
        entire_region.width, entire_region.height,
        graphics::render_target::rendering_method::cpu));
    (*render_target)->BeginDraw();
    (*render_target)->Clear(D2D1::ColorF(GetRValue(bg_color) / 255., GetGValue(bg_color) / 255., GetBValue(bg_color) / 255., 1.0f));
    (*render_target)->EndDraw();
    for (auto& view : views){
        view->prepare();
    }
    cover_window->start(entire_region, region);
    views[current_view]->show();

    std::ostringstream os;
    os << "mapper-core: Start viewport [" << name << "] [";
    os << entire_region.width << " x " << entire_region.height << "]";
    mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, os.str());
}

void ViewPort::disable(){
    if (is_enable) {
        is_enable = false;
        views[current_view]->hide();
        cover_window->stop();
        render_target = nullptr;
    }
}

void ViewPort::process_touch_event(){
    if (is_enable){
        std::lock_guard lock(touch_event_mutex);
        for (auto i = 0; i < touch_event_num; i++){
            auto& slot = touch_event_buffer[i];
            views[current_view]->process_touch_event(slot.event, slot.x, slot.y);
        }
        touch_event_num = 0;
    }
}

void ViewPort::update(){
    if (is_enable){
        views[current_view]->update_view();
    }
}

void ViewPort::invaridate_rect(const FloatRect& rect){
    if (is_enable){
        std::lock_guard lock(rendering_mutex);
        (*render_target)->BeginDraw();
        (*render_target)->PushAxisAlignedClip(entire_region_client, D2D1_ANTIALIAS_MODE_ALIASED);
        (*render_target)->PushAxisAlignedClip(region_client, D2D1_ANTIALIAS_MODE_ALIASED);
        auto updated = views[current_view]->render_view(*render_target, rect);
        (*render_target)->PopAxisAlignedClip();
        (*render_target)->PopAxisAlignedClip();
        (*render_target)->EndDraw();
        if (updated && rendering_count == rendering_reflect_count){
            rendering_count++;
            cover_window->update_window();
        }
    }
}

int ViewPort::registerView(sol::object def_obj){
    return lua_c_interface(manager.get_engine(), "viewport:register_view", [this, &def_obj](){
        if (is_freezed){
            throw MapperException("Viewport definitions are fixed by calling mapper.start_viewports(). "
                                  "You may need to call mapper.reset_viewports().");
        }
        auto view = std::make_unique<View>(manager.get_engine(), *this, def_obj);
        mappings_num_for_views += view->getMappingsNum();
        views.push_back(std::move(view));
        manager.get_engine().notifyUpdate(MapperEngine::UPDATED_VIEWPORTS);
        manager.get_engine().notifyUpdate(MapperEngine::UPDATED_MAPPINGS);
        if (current_view == 0){
            current_view = 1;
        }
        return views.size() - 1;
    });
}

Action* ViewPort::findAction(uint64_t evid){
    if (is_enable){
        auto action = views[current_view]->findAction(evid);
        if (action){
            return action;
        }else if (mappings && mappings->count(evid)){
            return mappings->at(evid).get();
        }
    }
    return nullptr;
}

std::pair<int, int> ViewPort::getMappingsStat(){
    return {mappings ?  mappings->size() : 0, mappings_num_for_views};
}

bool ViewPort::findCapturedWindow(FloatPoint point, View::CapturedWindowAttributes& attrs){
    if (is_enable){
         return views[current_view]->findCapturedWindow(point, attrs);
    }else{
        return false;
    }
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
            if (current_view != *view_no && is_enable){
                auto prev = current_view;
                current_view = *view_no;
                views[current_view]->show();
                views[prev]->hide();
            }
        }else{
            throw MapperException("invalid view number is specified");
        }
    });
}

void ViewPort::setMappings(sol::object mapdef){
    lua_c_interface(manager.get_engine(), "viewport:set_mappings", [this, &mapdef](){
        mappings = std::move(createEventActionMap(manager.get_engine(), mapdef));
        manager.get_engine().notifyUpdate(MapperEngine::UPDATED_MAPPINGS);
    });
}

void ViewPort::addMappings(sol::object mapdef){
    lua_c_interface(manager.get_engine(), "viewport:add_mappings", [this, &mapdef](){
        if (!mappings){
            mappings = std::move(createEventActionMap(manager.get_engine(), mapdef));
        }else{
            addEventActionMap(manager.get_engine(), mappings, mapdef);
        }
        manager.get_engine().notifyUpdate(MapperEngine::UPDATED_MAPPINGS);
    });
}

//============================================================================================
// Viewport manager inmplementation
//============================================================================================
ViewPortManager::ViewPortManager(MapperEngine& engine) : engine(engine){
    if (the_manager){
        abort();
    }
    hookdll_startGlobalHook(&ViewPortManager::notify_close_proc, this);
    mouse_emulator = std::move(mouse_emu::create_emulator());
    install_mouse_hook();
    the_manager = this;
}

ViewPortManager::~ViewPortManager(){
    the_manager = nullptr;    
    reset_viewports();
    mouse_emulator = nullptr;
    uninstall_mouse_hook();
    hookdll_stopGlobalHook();
}

void ViewPortManager::init_scripting_env(sol::table& mapper_table){
    //
    // functions to handle viewport
    //
    mapper_table.new_usertype<ViewPort>(
        "viewport",
        sol::call_constructor, sol::factories([this](sol::object def){
            return lua_c_interface(engine, "mapper.viewport", [this, &def](){
                return create_viewport(def);
            });
        }),
        "empty_view", sol::property([]{return 0;}),
        "current_view", sol::property(&ViewPort::getCurrentView),
        "change_view", &ViewPort::setCurrentView,
        "register_view", &ViewPort::registerView,
        "set_mappings", &ViewPort::setMappings,
        "add_mappings", &ViewPort::addMappings
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

    //
    // view element objects
    //
    viewobject_init_scripting_env(engine, mapper_table);
    sol::table view_elements = mapper_table["view_elements"];

    //
    // functions to handle captured window
    //
    mapper_table.new_usertype<CapturedWindow>(
        "captured_window",
        sol::call_constructor, sol::factories([this](sol::object def){
            return lua_c_interface(engine, "mapper.captured_window", [this, &def](){
                return create_captured_window(def);
            });
        })
    );
    view_elements.new_usertype<CapturedWindow>(
        "captured_window",
        sol::call_constructor, sol::factories([this](sol::object def){
            return lua_c_interface(engine, "mapper.view_elemnets.captured_window", [this, &def](){
                return create_captured_window(def);
            });
        })
    );
}

Action* ViewPortManager::find_action(uint64_t evid){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        for (auto& viewport : viewports){
            auto action = viewport->findAction(evid);
            if (action){
                return action;
            }
        }
    }
    return nullptr;
}

bool ViewPortManager::findCapturedWindow(FloatPoint point, View::CapturedWindowAttributes& attrs){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        for (auto& viewport : viewports){
            if (viewport->findCapturedWindow(point, attrs)){
                return true;
            }
        }
    }
    return false;
}

void ViewPortManager::process_touch_event(){
    if (status == Status::running){
        for (auto& viewport : viewports){
            viewport->process_touch_event();
        }
    }
}

void ViewPortManager::update_viewports(){
    if (status == Status::running){
        for (auto& viewport : viewports){
            viewport->update();
        }
    }
}

std::shared_ptr<ViewPort> ViewPortManager::create_viewport(sol::object def_obj){
    std::lock_guard lock(mutex);
    if (status == Status::init){
        auto viewport = std::make_shared<ViewPort>(*this, def_obj);
        viewports.push_back(viewport);
        engine.notifyUpdate(engine.UPDATED_VIEWPORTS);
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

        if (captured_windows.size() > 0){
            change_status(Status::ready_to_start);
            lock.unlock();
            engine.notifyUpdate(MapperEngine::UPDATED_READY_TO_CAPTURE);
            return;
        }

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
        engine.notifyUpdate(MapperEngine::UPDATED_VIEWPORTS_STATUS);
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
        engine.notifyUpdate(MapperEngine::UPDATED_VIEWPORTS_STATUS);
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

    for (auto& item: captured_windows){
        item.second->release_window();
    }
    viewports.clear();
    captured_windows.clear();
    engine.recommend_gc();

    auto prev_status = status;
    change_status(Status::init);
    lock.unlock();
    if (prev_status != Status::init) {
        engine.notifyUpdate(MapperEngine::UPDATED_VIEWPORTS_STATUS);
        engine.notifyUpdate(MapperEngine::UPDATED_MAPPINGS);
    }
    engine.recommend_gc();
}

std::shared_ptr<CapturedWindow> ViewPortManager::create_captured_window(sol::object def_obj){
    std::lock_guard lock(mutex);
    if (status == Status::init){
        auto cwid = cwid_counter++;
        auto captured_window = std::make_shared<CapturedWindow>(engine, cwid, def_obj);
        captured_windows.emplace(cwid, captured_window);
        return captured_window;
    }else{
        throw MapperException("Captured window object cannot create since viewport definitions are fixed "
                              "by calling mapper.start_viewports(). "
                              "You need to call mapper.reset_viewports() before creating new captured window object.");
    }
}

ViewPortManager::cw_info_list ViewPortManager::get_captured_window_list(){
    std::lock_guard lock(mutex);
    cw_info_list list;
    for (auto& cw : captured_windows){
        CapturedWindowInfo cwinfo = {cw.first, cw.second->get_name(), cw.second->get_hwnd() != 0};
        list.push_back(std::move(cwinfo));
    }
    return std::move(list);
}

void ViewPortManager::register_captured_window(uint32_t cwid, HWND hWnd){
    std::lock_guard lock(mutex);
    if (captured_windows.count(cwid) > 0){
        auto& cw = captured_windows.at(cwid);
        if (cw->get_hwnd() && cw->get_hwnd() != hWnd){
            throw MapperException("specified captured window is already associate with a window");
        }
        cw->attach_window(hWnd);
    }else{
        throw MapperException("specified captured window no longer exits");
    }
}

void ViewPortManager::unregister_captured_window(uint32_t cwid){
    std::lock_guard lock(mutex);
    if (captured_windows.count(cwid) > 0){
        auto& cw = captured_windows.at(cwid);
        cw->release_window();
    }else{
        throw MapperException("specified captured window no longer exits");
    }
}

ViewPortManager::vp_info_list ViewPortManager::get_viewport_list(){
    std::lock_guard lock(mutex);
    vp_info_list list;
    for (auto& viewport : viewports){
        std::vector<std::string> views;
        for (auto& view : viewport->views){
            views.emplace_back(view->name);
        }
        list.emplace_back(viewport->name.c_str(), std::move(views));
    }
    return list;
}

void ViewPortManager::enable_viewports(){
    std::unique_lock lock(mutex);
    if (status == Status::ready_to_start){
        change_status(Status::starting);
        lock.unlock();
        try{
            enable_viewport_primitive();
        }catch (MapperException& e){
            lock.lock();
            change_status(Status::ready_to_start);
            throw e;
        }
        lock.lock();
        change_status(Status::running);
        lock.unlock();
        engine.notifyUpdateWithNoLock(MapperEngine::UPDATED_VIEWPORTS_STATUS);
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
        engine.notifyUpdateWithNoLock(MapperEngine::UPDATED_VIEWPORTS_STATUS);
    }
}

void ViewPortManager::enable_viewport_primitive(){	
    displays.clear();
    ::EnumDisplayMonitors(nullptr, nullptr, ViewPortManager::monitor_enum_proc, reinterpret_cast<LPARAM>(this));
    int i = 0;
    try{
        for (i = 0; i < viewports.size(); i++){
            viewports[i]->enable(displays);
        }
    }catch(MapperException& e){
        for (int j = 0; j < i; j++){
            viewports[j]->disable();
        }
        throw e;
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

void ViewPortManager::notify_close_proc(HWND hWnd, void* context){
    auto self = reinterpret_cast<ViewPortManager*>(context);
    self->process_close_event(hWnd);
}

void ViewPortManager::process_close_event(HWND hWnd){
    std::unique_lock lock(mutex);
    for (auto item : captured_windows){
        if (item.second->get_hwnd() == hWnd){
            item.second->release_window();
            auto cwid = item.first;
            lock.unlock();
            engine.notifyUpdate(MapperEngine::UPDATED_LOST_CAPTURED_WINDOW);
            return;
        }
    }
}

std::pair<int, int> ViewPortManager::get_mappings_stat(){
    std::lock_guard lock{mutex};
    int for_viewports{0};
    int for_views{0};
    for (auto& viewport : viewports){
        auto&& stat = viewport->getMappingsStat();
        for_viewports += stat.first;
        for_views += stat.second;
    }
    return {for_viewports, for_views};
}