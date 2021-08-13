//
// simplewindow.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "simplewindow.h"

//============================================================================================
// constract / destract instance
//============================================================================================
WinBase::WinBase(){
	req_destroy_msg = ::RegisterWindowMessageA("MAPPER_CORE_REQ_DESTROY_WINDOW");
}

WinBase::~WinBase()
{
	destroy();
}

//============================================================================================
// Window creation / delstraction
//============================================================================================
bool WinBase::create(){
	{
		std::lock_guard lock(mutex);
		if (status == Status::init){
			status = Status::creating;
			cv.notify_all();
		}else{
			return false;
		}
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
			std::lock_guard lock(mutex);
			status = Status::init;
			cv.notify_all();
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

	messaging_thread = std::move(std::thread([this, &cs](){
		hWnd = ::CreateWindowExA(
			cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style,
			cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent,
			cs.hMenu, cs.hInstance, cs.lpCreateParams);
		
		{
			std::lock_guard lock(mutex);
			status = Status::created;
			cv.notify_all();
		}

		MSG msg;
		while( 0 != ::GetMessageA(&msg, NULL, 0, 0)){
			::TranslateMessage(&msg);
			::DispatchMessageA(&msg);
		}
	}));

	std::unique_lock lock(mutex);
	cv.wait(lock, [this](){return status != Status::creating;});

	return hWnd;
}

bool WinBase::destroy(){
	std::unique_lock lock(mutex);
	if (status == Status::init){
		return true;
	}else if (status == Status::creating){
		cv.wait(lock, [this](){return status == Status::created;});
	}else if (status == Status::deleting){
		cv.wait(lock, [this](){return status == Status::init;});
		return true;
	}

	status = Status::deleting;
	PostMessageA(hWnd, req_destroy_msg, 0, 0);

	lock.unlock();
	messaging_thread.join();
	lock.lock();

	status = Status::init;
	hWnd = nullptr;
	cv.notify_all();

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
			::PostQuitMessage(0);
			return 0;
		}else{
			return -1;
		}
	}else if (uMsg == WM_PAINT){
		onPaint();
		return 0;
	}else if (uMsg == req_destroy_msg){
		::DestroyWindow(hWnd);
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
