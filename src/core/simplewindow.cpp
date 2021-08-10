//
// simplewindow.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "simplewindow.h"

//============================================================================================
// destract instance
//============================================================================================
WinBase::~WinBase()
{
	if (hWnd){
		destroy();
	}
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

		WNDCLASSA wc;
		wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WinBase::windowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(WinBase*);
		wc.hInstance = hModule;
		wc.hIcon = nullptr;
		wc.hCursor = nullptr;
		wc.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = nullptr;
	
		preRegisterClass(wc);
		
		ATOM atom = ::RegisterClassA(&wc);
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

	hWnd = ::CreateWindowExA(
		cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style,
		cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent,
		cs.hMenu, cs.hInstance, cs.lpCreateParams);

	return hWnd;
}

bool WinBase::destroy(){
	if (hWnd && !::DestroyWindow(*this)){
		return false;
	}
    hWnd = nullptr;
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
// Message procedure
//============================================================================================
LRESULT WinBase::messageProc(UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg){
    case WM_DESTROY:
        return onDestroy() ? 0 : -1;
    case WM_PAINT:
        onPaint();
        return 0;
    default:
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

//============================================================================================
// Functions that respond to each message
//============================================================================================
bool WinBase::onCreate(CREATESTRUCT* pcs){
	::DefWindowProc(*this, WM_CREATE, 0, reinterpret_cast<LPARAM>(pcs));
	return true;
}

bool WinBase::onDestroy(){
	return true;
}

void WinBase::onPaint(){
	::DefWindowProc(*this, WM_PAINT, 0, 0);
}
