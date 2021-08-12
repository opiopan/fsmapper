#include <iostream>
#include <mutex>
#include <functional>
#include <thread>
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
    return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
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


static bool console_handler(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char *msg, size_t len){
    static std::mutex mutex;
    std::lock_guard lock(mutex);
    std::cout.write(msg, len);
    std::cout << std::endl;
    return true;
}

static bool event_handler(MapperHandle mapper, MAPPER_EVENT ev, int64_t data){
    return true;
}

int main(int argc, char* argv[]){
    if (argc < 2){
        std::cerr << "usage: " << argv[0] << "script-path" << std::endl;
        return 1;
    }

    auto hWnd = create_main_window();

    auto thread = std::thread([argv, hWnd](){
        MapperHandle mapper = mapper_init(event_handler, console_handler, nullptr);
        mapper_setLogMode(mapper, MAPPER_LOG_EVENT);
        //mapper_setLogMode(mapper, 0);
        auto rc = mapper_run(mapper, argv[1]);
        mapper_terminate(mapper);
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
