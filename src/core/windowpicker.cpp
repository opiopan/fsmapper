//
// windowpicker.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <vector>
#include <sstream>
#include "mappercore.h"
#include "tools.h"
#include "encoding.hpp"
#include "simplewindow.h"

#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>
#include <dwmapi.h>

constexpr auto MAX_ENABLE_WINDOW_RATIO = 50;

class WindowPicker;

//============================================================================================
// Window that covers entire of each display
//============================================================================================
static constexpr auto cover_window_class_name = "mapper_picking_window";
class CoverWindow : public SimpleWindow<cover_window_class_name>{
protected:
    WindowPicker& picker;
    IntRect rect;
    GdiObject<HBITMAP> bitmap;
    void* bitmap_data {nullptr};
    std::optional<IntRect> hilighting {std::nullopt};
public:
    CoverWindow(const WinDispatcher& dispatcher, WindowPicker& picker, IntRect& rect): 
        SimpleWindow(dispatcher), picker(picker), rect(rect){
    }
    ~CoverWindow(){}

    void start(){
        create();
        showWindow(SW_SHOW);
    }

    void stop(){
        destroy();
    }

    void update_hilighting(std::optional<IntRect> selected_rect){
        if (selected_rect){
            auto&& valid = rect.intersect(*selected_rect);
            if (valid.width == 0 || valid.height == 0){
                if (hilighting){
                    hilighting = std::nullopt;
                }else{
                    return;
                }
            }else{
                hilighting = selected_rect;
            }
        }else{
            if (hilighting){
                hilighting = selected_rect;
            }else{
                return;
            }
        }
        update_window();
    }

protected:
    LRESULT messageProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

    void preCreateWindow(CREATESTRUCTA& cs) override{
        SimpleWindow::preCreateWindow(cs);
        cs.dwExStyle = WS_EX_LAYERED | WS_EX_TOPMOST;
        cs.x = rect.x;
        cs.y = rect.y;
        cs.cx = rect.width;
        cs.cy = rect.height;
    }

    bool onCreate(CREATESTRUCT* cs) override{
        SimpleWindow::onCreate(cs);
        BITMAPV4HEADER hdr = {0};
        hdr.bV4Size = sizeof(BITMAPV4HEADER);
        hdr.bV4Width = rect.width;
        hdr.bV4Height = rect.height;
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
        WinDC dc{nullptr};
        bitmap = ::CreateDIBSection(
            dc, reinterpret_cast<BITMAPINFO*>(&hdr), DIB_RGB_COLORS, &bitmap_data, nullptr, 0);

        update_window();
        return true;
    }

    void update_window();
};

//============================================================================================
// Object to manage window picking
//============================================================================================
class WindowPicker{
protected:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed{false};
    HWND app_wnd;
    tools::utf8_to_utf16_translator target_name;
    std::vector<IntRect> displays;
    std::vector<std::unique_ptr<CoverWindow>> windows;
    HWND target{nullptr};
    struct WindowDef{
        HWND hwnd;
        IntRect rect;
        std::string title;
        WindowDef()=delete;
        WindowDef(HWND hwnd, const IntRect& rect, const char* title): hwnd(hwnd), rect(rect), title(title){}
    };
    std::vector<WindowDef> window_defs;
    
public:
    WindowPicker(HWND app_wnd, const char* target_name) : app_wnd(app_wnd), target_name(target_name){
        //
        // Building a cover window for each display monitor
        //
        ::EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR, HDC, LPRECT rect, LPARAM context)->BOOL{
            auto self = reinterpret_cast<WindowPicker*>(context);
            self->displays.emplace_back(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
            auto window = std::make_unique<CoverWindow>(WinDispatcher::sharedDispatcher(), *self, *self->displays.rbegin());
            self->windows.emplace_back(std::move(window));
            return true;
        }, reinterpret_cast<LPARAM>(this));

        //
        // Building window list wich can be select
        // This is based on dirty huristic rules:
        //    - exclude windows that width / height ratio is huge
        //    - exclude all layered window since I couldn't find a way to find actual non visible layered window
        //    - exclude same geometory with desktop window
        //
        auto desktop = ::GetDesktopWindow();
        RECT temp_rect;
        ::GetWindowRect(desktop, &temp_rect);
        IntRect desktop_rect = {
            temp_rect.left, temp_rect.top,
            temp_rect.right - temp_rect.left,
            temp_rect.bottom - temp_rect.top
        };
        auto window = ::GetTopWindow(nullptr);
        while ((window = ::GetWindow(window, GW_HWNDNEXT)) != nullptr && window != desktop){
            auto is_visible = ::IsWindowVisible(window);
            int is_cloaked;
            ::DwmGetWindowAttribute(window, DWMWA_CLOAKED, &is_cloaked, sizeof(is_cloaked));
            WINDOWINFO info;
            ::GetWindowInfo(window, &info);
            IntRect rect{info.rcWindow.left, info.rcWindow.top, 
                         info.rcWindow.right - info.rcWindow.left,
                         info.rcWindow.bottom - info.rcWindow.top};
            auto is_layered = info.dwExStyle & WS_EX_LAYERED;
            if (is_visible && !is_cloaked && !is_layered &&
                window != app_wnd &&
                rect.width > 0 && rect.height > 0 &&
                max(rect.width, rect.height) / min(rect.width, rect.height) < MAX_ENABLE_WINDOW_RATIO){
                if (desktop_rect == rect || 
                    (rect.width >= desktop_rect.width && rect.height >= desktop_rect.height)){
                    break;
                }
                char buf[512];
                ::GetWindowTextA(window, buf, sizeof(buf));
                window_defs.emplace_back(window, rect, buf);
            }
        }
    }

    ~WindowPicker(){}

    HWND pick_window(){
        for (auto& window : windows){
            window->start();
        }
        std::unique_lock lock(mutex);
        cv.wait(lock, [this](){return completed;});
        return target;
    }

    void update_target(POINT& point){
        for (auto& def : window_defs){
            if (def.rect.pointIsInRectangle(point.x, point.y)){
                if (def.hwnd != target){
                    target = def.hwnd;
                    for (auto& window : windows){
                        window->update_hilighting(def.rect);
                    }
                }
                return;
            }
        }
        if (target){
            target = nullptr;
            for (auto& window : windows){
                window->update_hilighting(std::nullopt);
            }
        }
    }

    void finish_picking(bool with_cancel = false){
        for (auto& window : windows){
            window->stop();
        }
        if (with_cancel){
            target = nullptr;
        }
        std::lock_guard lock(mutex);
        completed = true;
        cv.notify_all();
    }

    const wchar_t* get_target_name(){return target_name;}
};


//============================================================================================
// functions of CoverWindow which must be defined after WindowPicker definition
//============================================================================================
LRESULT CoverWindow::messageProc(UINT msg, WPARAM wparam, LPARAM lparam){
    if (msg == WM_MOUSEMOVE){
        auto x = LOWORD(lparam);
        auto y = HIWORD(lparam);
        POINT point {x, y};
        ::ClientToScreen(hWnd, &point);
        picker.update_target(point);
    }else if (msg == WM_LBUTTONUP){
        picker.finish_picking();
    }else if (msg == WM_RBUTTONUP){
        picker.finish_picking(true);
    }else if (msg == WM_KEYDOWN){
        if (wparam == VK_ESCAPE){
            picker.finish_picking(true);
        }
    }else{
        return SimpleWindow::messageProc(msg, wparam, lparam);
    }
    return 0;
}

void CoverWindow::update_window(){
    WinDC dc{nullptr};
    MemDC memdc{dc};
    auto prev_bitmap = ::SelectObject(memdc, bitmap.get_handle());

    Gdiplus::Graphics graphics(memdc);
    graphics.Clear(Gdiplus::Color(127, 0, 0, 0));

    //
    // draw the region of window under the mouse pointer
    //
    if (hilighting){
        Gdiplus::Pen pen(Gdiplus::Color::Yellow, 32.0);
        pen.SetAlignment(Gdiplus::PenAlignment::PenAlignmentInset);
        graphics.DrawRectangle(
            &pen, hilighting->x - rect.x, hilighting->y - rect.y, hilighting->width, hilighting->height);
    }

    //
    // draw notice message at the center of display
    //
    {
        auto font_height = static_cast<Gdiplus::REAL>(rect.width * 0.04);
        Gdiplus::FontFamily font_family{ L"Segoe UI" };
        Gdiplus::Font font{ &font_family, font_height, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel };
        Gdiplus::RectF layout_rect{ 0., 0., static_cast<Gdiplus::REAL>(rect.width), static_cast<Gdiplus::REAL>(rect.height) };

        auto str1 = L"Select a window to capture";
        Gdiplus::RectF str1_rect;
        graphics.MeasureString(str1, -1, &font, layout_rect, &str1_rect);

        std::wostringstream os;
        os << L"for \"" << picker.get_target_name() << L"\"";
        auto&& str2 = os.str();
        Gdiplus::RectF str2_rect;
        graphics.MeasureString(str2.c_str(), -1, &font, layout_rect, &str2_rect);

        auto str3 = L"or right click to cancel";
        Gdiplus::RectF str3_rect;
        graphics.MeasureString(str3, -1, &font, layout_rect, &str3_rect);

        auto spacing = font_height * 0.0;          
        Gdiplus::PointF point1{
            static_cast<Gdiplus::REAL>((layout_rect.Width - str1_rect.Width) / 2.0),
            static_cast<Gdiplus::REAL>((layout_rect.Height - str1_rect.Height - str2_rect.Height - str3_rect.Height - spacing) / 2.0) };
        Gdiplus::PointF point2{
            static_cast<Gdiplus::REAL>((layout_rect.Width - str2_rect.Width) / 2.0),
            static_cast<Gdiplus::REAL>(point1.Y + str1_rect.Height + spacing) };
        Gdiplus::PointF point3{
            static_cast<Gdiplus::REAL>((layout_rect.Width - str3_rect.Width) / 2.0),
            static_cast<Gdiplus::REAL>(point2.Y + str2_rect.Height + spacing) };
        Gdiplus::SolidBrush brush(Gdiplus::Color(130, 0, 255, 255));
        graphics.DrawString(str1, -1, &font, point1, &brush);
        graphics.DrawString(str2.c_str(), -1, &font, point2, &brush);
        graphics.DrawString(str3, -1, &font, point3, &brush);
    }

    POINT position{rect.x, rect.y};
    static POINT point{ 0, 0 };
    SIZE size;
    size.cx = rect.width;
    size.cy = rect.height;
    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    ::UpdateLayeredWindow(*this, dc, &position, &size, memdc, &point, 0, &blend, ULW_ALPHA);

    ::SelectObject(memdc, prev_bitmap);
};

//============================================================================================
// exported function as interface of DLL
//============================================================================================
DLLEXPORT HWND mapper_tools_PickWindow(HWND app_wnd, const char* name){
    WindowPicker picker{app_wnd, name};
    return picker.pick_window();
}
