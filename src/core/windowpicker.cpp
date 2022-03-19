//
// windowpicker.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <vector>
#include "mappercore.h"
#include "simplewindow.h"
#include "viewport.h"

class WindowPicker;

//============================================================================================
// Window that covers entire of each display
//============================================================================================
static constexpr auto cover_window_class_name = "mapper_picking_window";
class CoverWindow : public SimpleWindow<cover_window_class_name>{
protected:
    WindowPicker& picker;
    IntRect rect;
public:
    CoverWindow(const WinDispatcher& dispatcher, WindowPicker& picker, IntRect& rect): 
        SimpleWindow(dispatcher), picker(picker), rect(rect){}
    ~CoverWindow(){}

    void start(){
        create();
        showWindow(SW_SHOW);
    }

    void stop(){
        destroy();
    }

protected:
	void preCreateWindow(CREATESTRUCTA& cs) override{
        SimpleWindow::preCreateWindow(cs);
        //cs.dwExStyle = WS_EX_LAYERED;
        cs.x = rect.x;
        cs.y = rect.y;
        cs.cx = rect.width;
        cs.cy = rect.height;
    }

    LRESULT messageProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
};

//============================================================================================
// Object to manage window picking
//============================================================================================
class WindowPicker{
protected:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed{false};
    WinDispatcher dispatcher;
    std::vector<IntRect> displays;
    std::vector<std::unique_ptr<CoverWindow>> windows;
    HWND target{nullptr};
    
public:
    WindowPicker(){
        ::EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR, HDC, LPRECT rect, LPARAM context)->BOOL{
            auto self = reinterpret_cast<WindowPicker*>(context);
            self->displays.emplace_back(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
            auto window = std::make_unique<CoverWindow>(self->dispatcher, *self, *self->displays.rbegin());
            self->windows.emplace_back(std::move(window));
            return true;
        }, reinterpret_cast<LPARAM>(this));
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
    }

    void finish_picking(){
        for (auto& window : windows){
            window->stop();
        }
    }
};


//============================================================================================
// functions of CoverWindow which must be defined after WindowPicker definition
//============================================================================================
LRESULT CoverWindow::messageProc(UINT msg, WPARAM wparam, LPARAM lparam){
    if (msg == WM_LBUTTONUP){
        picker.finish_picking();
        return 0;
    }else if (msg == WM_MOVE){
        auto x = LOWORD(lparam);
        auto y = HIWORD(lparam);
        POINT point {x, y};
        ::ClientToScreen(hWnd, &point);
        picker.update_target(point);
    }else{
        return SimpleWindow::messageProc(msg, wparam, lparam);
    }
}

//============================================================================================
// exported function as interface of DLL
//============================================================================================
DLLEXPORT HWND mapper_tools_PickWindow(){
    WindowPicker picker;
    return picker.pick_window();
}
