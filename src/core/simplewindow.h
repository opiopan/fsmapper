//
// simplewindow.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>

//============================================================================================
// Root class that represent window
//     No incetance will be created from this class directry 
//     since this class contains pure virtual functions
//============================================================================================
class WinBase{
protected:
	HWND hWnd = nullptr;
	
public:
	WinBase() = default;
    WinBase(const WinBase&) = delete;
    WinBase(WinBase&&) = delete;
	virtual ~WinBase();
    WinBase& operator = (const WinBase&)const = delete;
    WinBase& operator = (WinBase&&)const = delete;

	bool create();
	bool destroy();

	bool showWindow(int nCmdShow){
		return ::ShowWindow(hWnd, nCmdShow);
	};

	operator HWND(){return hWnd;};

protected:
	static LRESULT CALLBACK windowProc(
		HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	virtual LRESULT messageProc(
		UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	virtual bool isRegisterd() = 0;
	virtual void noticeRegisterd(ATOM atom) = 0;
	virtual void preRegisterClass(WNDCLASSA& wc) = 0;
	virtual void preCreateWindow(CREATESTRUCTA& cs) = 0;

	virtual bool onCreate(CREATESTRUCT* pcs);
	virtual bool onDestroy();
	virtual void onPaint();
};

//============================================================================================
// Simple window representaion inherited from WinBase
//============================================================================================
struct WindowClassName{
	const char* name;
	operator const char* () const{return name;};
};

template <const WindowClassName& class_name, LONG style = WS_OVERLAPPED>
class SimpleWindow : public WinBase{
protected:
    static ATOM class_atom;

public:
	SimpleWindow() = default;
	virtual ~SimpleWindow() = default;

protected:
	virtual bool isRegisterd() override {return class_atom;};
	virtual void noticeRegisterd(ATOM atom) override {class_atom = atom;};
	virtual void preRegisterClass(WNDCLASSA& wc) override {wc.lpszClassName = class_name;};
	virtual void preCreateWindow(CREATESTRUCTA& cs) override{
        cs.style = style;
        cs.lpszClass = class_name;
    };
};

template <const WindowClassName& class_name, LONG style>
ATOM SimpleWindow<class_name, style>::class_atom = 0;