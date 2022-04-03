//
// viewport.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <stdexcept>
#include <cfenv>
#include <cmath>
#include "viewport.h"
#include "engine.h"
#include "capturedwindow.h"
#include "tools.h"
#include "hookdll.h"

#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>

//============================================================================================
// View inmplementation
//============================================================================================
ViewPort::View::View(MapperEngine& engine, sol::object& def_obj){
    if (def_obj.get_type() != sol::type::table){
        throw MapperException("view definition must be specified as a table");
    }
    auto def = def_obj.as<sol::table>();

    name = std::move(lua_safestring(def["name"]));
    if (name.length() == 0){
        throw MapperException("\"name\" parameter for view definition must be specified");
    }

    sol::object elements_val = def["elements"];
    if (elements_val.get_type() == sol::type::table){
        sol::table elements_def = elements_val;
        for (int i = 1; i <= elements_def.size(); i++){
            auto item = elements_def[i];
            FloatRect region(0, 0, 1, 1);
            bool is_relative = true;
            
            auto&& region_type = lua_safevalue<std::string>(item["region_type"]);
            if (region_type && region_type.value() == "relative"){
                is_relative = true;
            }else if (region_type && region_type.value() == "absolute"){
                is_relative = false;
            }else if (region_type){
                throw MapperException("Invalid value for \"region_type\" parameter, \"relative\" or \"absolute\" can be specified");
            }

            auto x = lua_safevalue<float>(item["x"]);
            auto y = lua_safevalue<float>(item["y"]);
            auto width = lua_safevalue<float>(item["width"]);
            auto height = lua_safevalue<float>(item["height"]);

            if (x && y && width && height){
                region = {*x, *y, *width, *height};
            }else if (!is_relative){
                throw MapperException("View element region must be specified by \"x\", \"y\", \"width\" and \"height\" parameters "
                                      "if absolute coordinate system is selected.");
            }

            sol::object object = item["object"];
            if (object.is<CapturedWindow&>()){
                auto element = std::make_unique<CWViewElement>(region, is_relative, object.as<CapturedWindow&>());
                captured_window_elements.push_back(std::move(element));
            }else{
                throw MapperException("unsupported object is specified as view element object");
            }
        }
    }
    if (captured_window_elements.size() == 0 && normal_elements.size() == 0){
        throw MapperException("there is no view element difinition, at least one view element is required");
    }

    mappings = std::move(createEventActionMap(engine, def["mappings"]));
}

ViewPort::View::~View(){
}

void ViewPort::View::show(ViewPort& viewport){
    for (auto& element : captured_window_elements){
        FloatRect region;
        element->transform_to_output_region(viewport.get_output_region(), region);
        IntRect iregion(std::roundf(region.x), std::roundf(region.y), std::roundf(region.width), std::roundf(region.height));
        element->get_object().change_window_pos(iregion, HWND_TOP, true, viewport.get_background_clolor());
    }
}

void ViewPort::View::hide(ViewPort& viewport){
    for (auto& element : captured_window_elements){
        FloatRect region;
        element->transform_to_output_region(viewport.get_output_region(), region);
        IntRect iregion(std::roundf(region.x), std::roundf(region.y), std::roundf(region.width), std::roundf(region.height));
        element->get_object().change_window_pos(iregion,HWND_BOTTOM, false);
    }
}

HWND ViewPort::View::getBottomWnd(){
    if (captured_window_elements.size() > 0){
        return captured_window_elements[captured_window_elements.size() -1 ]->get_object().get_hwnd();
    }else{
        return nullptr;
    }
}

Action* ViewPort::View::findAction(uint64_t evid){
    if (mappings && mappings->count(evid)){
        return mappings->at(evid).get();
    }else{
        return nullptr;
    }
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
};

template <typename MSG_RELAY>
class ViewPortWindow : public ViewPort::CoverWindow{
protected:
    using base_class = ViewPort::CoverWindow;
    MSG_RELAY relay;
    COLORREF bgcolor;
    IntRect varid_rect;
    IntRect entire_rect;
    GdiObject<HBITMAP> bitmap;
    void* bitmap_data {nullptr};

public:
    ViewPortWindow(COLORREF bgcolor, const MSG_RELAY& relay): bgcolor(bgcolor), relay(relay){};
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

protected:
    LRESULT messageProc(UINT msg, WPARAM wparam, LPARAM lparam) override{
        if (msg == WM_MOUSEMOVE || 
            msg == WM_LBUTTONUP || msg == WM_LBUTTONDOWN || 
            msg == WM_RBUTTONUP || msg == WM_RBUTTONDOWN){
            return relay(msg, wparam, lparam);
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
        BITMAPV4HEADER hdr = {0};
        hdr.bV4Size = sizeof(BITMAPV4HEADER);
        hdr.bV4Width = entire_rect.width;
        hdr.bV4Height = entire_rect.height;
        hdr.bV4Planes = 1;
        hdr.bV4BitCount = 32;
        hdr.bV4V4Compression = BI_BITFIELDS;
        hdr.bV4SizeImage = 0;
        hdr.bV4XPelsPerMeter = 0;
        hdr.bV4YPelsPerMeter = 0;
        hdr.bV4ClrUsed = 0;
        hdr.bV4ClrImportant = 0;
        hdr.bV4RedMask = 0x00FF0000;
        hdr.bV4GreenMask = 0x0000FF00;
        hdr.bV4BlueMask = 0x000000FF;
        hdr.bV4AlphaMask = 0xFF000000;
        bitmap = ::CreateDIBSection(
            nullptr, reinterpret_cast<BITMAPINFO*>(&hdr), DIB_RGB_COLORS, &bitmap_data, nullptr, 0);
        update_window(true);
        return true;
    }

    void update_window(bool initial_update = false){
        WinDC dc{nullptr};
        MemDC memdc{dc};
        auto prev_bitmap = ::SelectObject(memdc, bitmap.get_handle());

        if (initial_update){
            Gdiplus::Graphics graphics(memdc);
            Gdiplus::RectF rect{
                static_cast<Gdiplus::REAL>(varid_rect.x), static_cast<Gdiplus::REAL>(varid_rect.y), 
                static_cast<Gdiplus::REAL>(varid_rect.width), static_cast<Gdiplus::REAL>(varid_rect.height)};
            graphics.ExcludeClip(rect);
            graphics.Clear(Gdiplus::Color(255, GetRValue(bgcolor), GetGValue(bgcolor), GetBValue(bgcolor)));
        }
        Gdiplus::Graphics graphics(memdc);
        Gdiplus::RectF erect{
            static_cast<Gdiplus::REAL>(entire_rect.x), static_cast<Gdiplus::REAL>(entire_rect.y),
            static_cast<Gdiplus::REAL>(entire_rect.width), static_cast<Gdiplus::REAL>(entire_rect.height)};
        Gdiplus::RectF vrect{
            static_cast<Gdiplus::REAL>(varid_rect.x), static_cast<Gdiplus::REAL>(varid_rect.y),
            static_cast<Gdiplus::REAL>(varid_rect.width), static_cast<Gdiplus::REAL>(varid_rect.height)};
        Gdiplus::Region region(erect);
        region.Exclude(vrect);
        graphics.ExcludeClip(&region);
        graphics.Clear(Gdiplus::Color(1, 0, 0, 0));

        POINT position{entire_rect.x, entire_rect.y};
        static POINT point{0, 0};
        SIZE size;
        size.cx = entire_rect.width;
        size.cy = entire_rect.height;
        BLENDFUNCTION blend;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;
        ::UpdateLayeredWindow(*this, dc, &position, &size, memdc, &point, 0, &blend, ULW_ALPHA);

        ::SelectObject(memdc, prev_bitmap);
    }
};

template <typename MSG_RELAY>
std::unique_ptr<ViewPortWindow<MSG_RELAY>> make_viewport_window(COLORREF bgcolor, const MSG_RELAY& relay){
    return std::move(std::make_unique<ViewPortWindow<MSG_RELAY>>(bgcolor, relay));
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

    aspect_ratio = lua_safevalue<float>(def["aspect_ratio"]);

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

    auto window = make_viewport_window(bg_color, [this](UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT{
        return 0;
    });
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
        entire_region.x = std::roundf(def_region.x);
        entire_region.y = std::roundf(def_region.y);
        entire_region.width = std::roundf(std::roundf(def_region.width));
        entire_region.height = std::roundf(def_region.height);
    }else{
        if (displays.size() < *def_display_no){
            std::ostringstream os;
            os << "Display number that is specified as viewport difinition is invalid. [viewport: " << name ;
            os << "] [display: " << *def_display_no << "]";
            throw MapperException(std::move(os.str()));
        }else{
            auto& drect = displays[*def_display_no - 1];
            if (is_relative_coordinates){
                entire_region.x = std::roundf(def_region.x * drect.width + drect.x);
                entire_region.y = std::roundf(def_region.y * drect.height + drect.y);
                entire_region.width = std::roundf(def_region.width * drect.width);
                entire_region.height = std::roundf(def_region.height * drect.height);
            }else{
                entire_region.x = std::roundf(def_region.x + drect.x);
                entire_region.y = std::roundf(def_region.y + drect.y);
                entire_region.width = std::roundf(def_region.width);
                entire_region.height = std::roundf(def_region.height);
            }
        }
    }
    if (aspect_ratio){
        auto eratio = static_cast<float>(entire_region.width) / static_cast<float>(entire_region.height);
        if (eratio > *aspect_ratio){
            auto factor = *aspect_ratio / eratio;
            region.width = entire_region.width * factor;
            region.x = entire_region.x + (entire_region.width - region.width) / 2;
            region.height = entire_region.height;
            region.y = entire_region.y;
        }else{
            auto factor = eratio / *aspect_ratio;
            region.width = entire_region.width;
            region.x = entire_region.x;
            region.height = entire_region.height * factor;
            region.y = entire_region.y + (entire_region.height - region.height) / 2;
        }
    }else{
        region = entire_region;
    }
    is_enable = true;
    cover_window->start(entire_region, region);
    views[current_view]->show(*this);
}

void ViewPort::disable(){
    if (is_enable) {
        is_enable = false;
        views[current_view]->hide(*this);
        cover_window->stop();
    }
}

int ViewPort::registerView(sol::object def_obj){
    return lua_c_interface(manager.get_engine(), "viewport:register_view", [this, &def_obj](){
        if (is_freezed){
            throw MapperException("Viewport definitions are fixed by calling mapper.start_viewports(). "
                                  "You may need to call mapper.reset_viewports().");
        }
        auto view = std::make_unique<View>(manager.get_engine(), def_obj);
        mappings_num_for_views += view->getMappingsNum();
        views.push_back(std::move(view));
        manager.get_engine().notifyUpdate(MapperEngine::UPDATED_VIEWPORTS);
        manager.get_engine().notifyUpdate(MapperEngine::UPDATED_MAPPINGS);
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
    return {mappings->size(), mappings_num_for_views};
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
                views[current_view]->show(*this);
                views[prev]->hide(*this);
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
        addEventActionMap(manager.get_engine(), mappings, mapdef);
        manager.get_engine().notifyUpdate(MapperEngine::UPDATED_MAPPINGS);
    });
}

//============================================================================================
// Viewport manager inmplementation
//============================================================================================
ViewPortManager::ViewPortManager(MapperEngine& engine) : engine(engine){
    hookdll_startGlobalHook(&ViewPortManager::notify_close_proc, this);
}

ViewPortManager::~ViewPortManager(){
    reset_viewports();
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
    }
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
        }catch (MapperException&){
            lock.lock();
            change_status(Status::ready_to_start);
            throw;
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
    for (auto& viewport : viewports){
        viewport->enable(displays);
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