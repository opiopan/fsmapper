//
// simplewindow.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "simplewindow.h"
#include <stdexcept>
#include <memory>

//============================================================================================
// Message pumping thread abstraction
//============================================================================================
static std::unique_ptr<WinDispatcher> shared_dispatcher;
void WinDispatcher::initSharedDispatcher(){
	if (!shared_dispatcher){
		shared_dispatcher = std::move(std::make_unique<WinDispatcher>());
	}
}
WinDispatcher& WinDispatcher::sharedDispatcher(){return *shared_dispatcher;}

ATOM WinDispatcher::class_atom {0};

WinDispatcher::WinDispatcher(){
	static const char* class_name = "WinDispatcher";
	invoke1_msg = ::RegisterWindowMessageA("MAPPER_CORE_INVOKE1");
	invoke2_msg = ::RegisterWindowMessageA("MAPPER_CORE_INVOKE2");
	HMODULE hModule = nullptr;
	::GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
		reinterpret_cast<LPCSTR>(WinDispatcher::windowProc),
		&hModule);
	if (!class_atom){
		WNDCLASSEXA wc ={0};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WinDispatcher::windowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(*this);
		wc.hInstance = hModule;
		wc.lpszClassName = class_name;
		class_atom = ::RegisterClassExA(&wc);
		if (!class_atom){
			throw std::runtime_error("failed to register window class for the dispatcher");
		}
	}
	dispatcher = std::move(std::thread([this, hModule](){
		{
			std::lock_guard lock(mutex);
			controller = ::CreateWindowExA(
				0,           // ExStyle
				class_name,  // Class
				nullptr,     // Name
				WS_POPUP,    // Style
				0, 0, 0, 0,  // Geometory
				nullptr,     // hwndParent
				nullptr,     // hMenu
				hModule,     // hInstance
				this);       // CreateParam
			cv.notify_all();
		}
		MSG msg;
		while(::GetMessageA(&msg, NULL, 0, 0) != 0){
			::TranslateMessage(&msg);
			::DispatchMessageA(&msg);
		}
	}));

	std::unique_lock lock(mutex);
	cv.wait(lock, [this](){return controller != 0;});
}

WinDispatcher::~WinDispatcher(){
	stop();
	dispatcher.join();
}

void WinDispatcher::stop(){
	std::lock_guard lock(mutex);
	if (controller){
		invoke([this](){
			::DestroyWindow(controller);
		});
		controller = nullptr;
	}
}

LRESULT CALLBACK WinDispatcher::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if (uMsg == WM_CREATE){
		auto pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
		auto win = reinterpret_cast<WinBase*>(pcs->lpCreateParams);
		::SetWindowLongPtrA(hWnd, 0, reinterpret_cast<LONG_PTR>(pcs->lpCreateParams));
		return 0;
	}else{
		auto self = reinterpret_cast<WinDispatcher*>(::GetWindowLongPtrA(hWnd, 0));
		if (self == nullptr){
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}else{
			if (uMsg == WM_DESTROY){
				::PostQuitMessage(0);
				return 0;
			}else if (uMsg == self->invoke1_msg){
				auto function = reinterpret_cast<dispatchable1*>(lParam);
				(*function)();
			}else if (uMsg == self->invoke2_msg){
				auto function = reinterpret_cast<dispatchable2*>(lParam);
				(*function)(reinterpret_cast<void*>(wParam));
			}else{
				return ::DefWindowProcA(hWnd, uMsg, wParam, lParam);
			}
		}
	}
}

//============================================================================================
// WinBase: constract / destract instance
//============================================================================================
WinBase::WinBase(const WinDispatcher& dispatcher) : dispatcher(dispatcher){
}

WinBase::~WinBase()
{
	destroy();
}

//============================================================================================
// Window creation / delstraction
//============================================================================================
bool WinBase::create(){
	if (hWnd){
		return false;
	}

	HMODULE hModule = nullptr;
	::GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
		reinterpret_cast<LPCSTR>(WinBase::windowProc), 
		&hModule);

	if (!isRegisterd()){
		WNDCLASSEXA wc;
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WinBase::windowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(WinBase*);
		wc.hInstance = hModule;
		wc.hIcon = nullptr;
		wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = nullptr;
		wc.hIconSm = nullptr;

		preRegisterClass(wc);
		
		ATOM atom = ::RegisterClassExA(&wc);
		if (atom == NULL){
			return false;
		}
		noticeRegisterd(atom);
	}

	CREATESTRUCTA cs;
	cs.lpCreateParams = this;
	cs.hInstance = hModule;
	cs.hMenu = nullptr;
	cs.hwndParent = nullptr;
	cs.cy = 0;
	cs.cx = 0;
	cs.y = 0;
	cs.x = 0;
	cs.style = 0;
	cs.lpszName = nullptr;
	cs.lpszClass = nullptr;
	cs.dwExStyle = 0;

	preCreateWindow(cs);

	dispatcher.invoke([this, cs](){
		hWnd = ::CreateWindowExA(
			cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style,
			cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent,
			cs.hMenu, cs.hInstance, cs.lpCreateParams);
	});

	return hWnd;
}

bool WinBase::destroy(){
	if (hWnd){
		dispatcher.invoke([this](){
			::DestroyWindow(hWnd);
		});
		hWnd = nullptr;
	}

	return true;
}

//============================================================================================
// Window procedure
//============================================================================================
LRESULT CALLBACK WinBase::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if (uMsg == WM_CREATE){
		auto pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
		auto win = reinterpret_cast<WinBase*>(pcs->lpCreateParams);
		::SetWindowLongPtrA(hWnd, 0, reinterpret_cast<LONG_PTR>(pcs->lpCreateParams));
		win->hWnd = hWnd;
		return win->onCreate(pcs) ? 0 : -1;
	}else{
		auto win = reinterpret_cast<WinBase*>(::GetWindowLongPtrA(hWnd, 0));
		if (win == nullptr)
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		else
			return win->messageProc(uMsg, wParam, lParam);
	}
}

//============================================================================================
// Message dispature
//============================================================================================
LRESULT WinBase::messageProc(UINT uMsg, WPARAM wParam, LPARAM lParam){
	if (uMsg == WM_DESTROY){
		if (onDestroy()){
			return 0;
		}else{
			return -1;
		}
	}else if (uMsg == WM_PAINT){
		onPaint();
		return 0;
	}else{
		return ::DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
}

//============================================================================================
// Functions that respond to each message
//============================================================================================
bool WinBase::onCreate(CREATESTRUCT* pcs){
	::DefWindowProcA(*this, WM_CREATE, 0, reinterpret_cast<LPARAM>(pcs));
	return true;
}

bool WinBase::onDestroy(){
	return true;
}

void WinBase::onPaint(){
	::DefWindowProcA(*this, WM_PAINT, 0, 0);
}
