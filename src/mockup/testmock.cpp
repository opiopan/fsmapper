#include <iostream>
#include <mutex>
#include <functional>
#include <thread>
#include <map>
#include <unordered_map>
#include <windows.h>
#include "mappercore.h"

static LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg) {
    case WM_CREATE:
        ::ShowWindow(hWnd, SW_HIDE);
        break;
    case WM_DESTROY:
        ::PostQuitMessage( 0 );
        break;
    }
    return ::DefWindowProcA( hWnd, uMsg, wParam, lParam );
}

static HWND create_main_window(){
    auto hInstance = ::GetModuleHandle(nullptr);

    WNDCLASSEXA tWndClass;
    tWndClass.cbSize        = sizeof(WNDCLASSEX);
    tWndClass.style         = CS_HREDRAW | CS_VREDRAW;
    tWndClass.lpfnWndProc   = window_proc;
    tWndClass.cbClsExtra    = 0;
    tWndClass.cbWndExtra    = 0;
    tWndClass.hInstance     = hInstance; 
    tWndClass.hIcon         = ::LoadIcon(nullptr, IDI_APPLICATION);
    tWndClass.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
    tWndClass.hbrBackground = reinterpret_cast<HBRUSH>( COLOR_WINDOW + 1 );
    tWndClass.lpszMenuName  = nullptr;
    tWndClass.lpszClassName = "testmock_main";
    tWndClass.hIconSm       = nullptr;
    if (::RegisterClassExA(&tWndClass) == 0) {
        return nullptr;
    }

    auto hWnd = ::CreateWindowExA(
        0                       // extended window style
        , tWndClass.lpszClassName // pointer to registered class name
        , "Test of mapper"        // pointer to window name
        , WS_OVERLAPPEDWINDOW     // window style
        , CW_USEDEFAULT           // horizontal position of window
        , CW_USEDEFAULT           // vertical position of window
        , 640                     // window width
        , 480                     // window height
        , nullptr                 // handle to parent or owner window
        , nullptr                 // handle to menu, or child-window identifier
        , hInstance               // handle to application instance
        , nullptr                 // pointer to window-creation data
    );

    return hWnd;
}

class MapperInterface{
protected:
    std::mutex mutex;
    std::condition_variable cv;
    std::thread controller;
    bool need_scan = false;
    bool should_stop = false;
    MapperHandle mapper = nullptr;

public:
    MapperInterface(){
        mapper = mapper_init(&MapperInterface::event_handler, &MapperInterface::console_handler, this);
        controller = std::move(std::thread([this](){
            while (true){
                std::unique_lock lock(mutex);
                cv.wait(lock, [this](){return need_scan || should_stop;});
                if (should_stop){
                    return;
                }else if (need_scan){
                    need_scan = false;
                    lock.unlock();
                    mapper_stopViewPort(mapper);
                    mapper_startViewPort(mapper);
                    lock.lock();
                }
            }
        }));
    }

    ~MapperInterface(){
        {
            std::lock_guard lock(mutex);
            should_stop = true;
            cv.notify_all();
        }
        controller.join();
        mapper_terminate(mapper);
    }

    bool run(const char* script_path){
        mapper_setLogMode(mapper, MAPPER_LOG_EVENT);
        //mapper_setLogMode(mapper, 0);
        return mapper_run(mapper, script_path);
    }

protected:
    static bool event_handler(MapperHandle mapper, MAPPER_EVENT ev, int64_t data){
        auto self = reinterpret_cast<MapperInterface*>(mapper_getHostContext(mapper));
        return self->process_event(ev, data);
    }

    bool process_event(MAPPER_EVENT ev, int64_t data){
        static const std::unordered_map<MAPPER_EVENT, const char*> evnames = {
            {MEV_CHANGE_SIMCONNECTION, "MEV_CHANGE_SIMCONNECTION"},
            {MEV_CHANGE_AIRCRAFT, "MEV_CHANGE_AIRCRAFT"},
            {MEV_CHANGE_DEVICES, "MEV_CHANGE_DEVICES"},
            {MEV_READY_TO_CAPTURE_WINDOW, "MEV_READY_TO_CAPTURE"},
            {MEV_LOST_CAPTURED_WINDOW, "MEV_LOCST_CAPTURED_WINDOW"},
            {MEV_START_VIEWPORTS, "MEV_START_VIEWPORTS"},
            {MEV_STOP_VIEWPORTS, "MEV_STOP_VIEWPORTS"},
            {MEV_RESET_VIEWPORTS, "MEV_RESET_VIEWPORTS"},
            {MEV_FAIL_SCRIPT, "MEV_FAIL_SCRIPT"},
        };

        std::lock_guard lock(mutex);
        std::cout << "=== " << evnames.at(ev) << " ===" << std::endl;

        if (ev == MEV_READY_TO_CAPTURE_WINDOW || ev == MEV_LOST_CAPTURED_WINDOW){
            need_scan = true;
            cv.notify_all();
        }

        return true;
    }

    static bool console_handler(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char *msg, size_t len){
        auto self = reinterpret_cast<MapperInterface*>(mapper_getHostContext(mapper));
        return self->process_console_message(type, msg, len);
    }

    bool process_console_message(MCONSOLE_MESSAGE_TYPE type, const char *msg, size_t len){
        std::lock_guard lock(mutex);
        std::cout.write(msg, len);
        std::cout << std::endl;
        return true;
    }
};

int main(int argc, char* argv[]){
    if (argc < 2){
        std::cerr << "usage: " << argv[0] << "script-path" << std::endl;
        return 1;
    }

    auto hWnd = create_main_window();

    auto thread = std::thread([argv, hWnd](){
        MapperInterface mapper;
        mapper.run(argv[1]);
        ::PostMessageA(hWnd, WM_CLOSE, 0, 0);
    });

    MSG msg;
    while( 0 != ::GetMessageA(&msg, NULL, 0, 0)){
        ::TranslateMessage(&msg);
        ::DispatchMessageA(&msg);
    }

    thread.join();
    return msg.wParam;
}
