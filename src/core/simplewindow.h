//
// simplewindow.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

//============================================================================================
// WinDispatcher: abstract message pump
//============================================================================================
class WinDispatcher{
protected:
	static ATOM class_atom;
	std::mutex mutex;
	std::condition_variable cv;
	std::thread dispatcher;
	HWND controller {0};
	UINT invoke1_msg {0};
	UINT invoke2_msg {0};
	using dispatchable1 = std::function<void()>;
	using dispatchable2 = std::function<void(void*)>;

public:
	WinDispatcher();
	WinDispatcher(const WinDispatcher&) = delete;
	WinDispatcher(WinDispatcher&&) = delete;
	~WinDispatcher();
	void stop();

	template <typename DISPATCHABLE>
	void invoke(DISPATCHABLE function) const{
		dispatchable1 function_obj{function};
		::SendMessageA(
			controller, invoke1_msg,
			0,
			reinterpret_cast<LPARAM>(&function_obj));
	}

	template <typename DISPATCHABLE>
	void invoke(DISPATCHABLE function, void* context) const{
		dispatchable2 function_obj{function};
		::SendMessageA(
			controller, invoke2_msg,
			reinterpret_cast<WPARAM>(context),
			reinterpret_cast<LPARAM>(&function_obj));
	}

	static void initSharedDispatcher();
	static WinDispatcher& sharedDispatcher();

protected:
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	
};

//============================================================================================
// Root class that represent window
//     No incetance will be created from this class directry 
//     since this class contains pure virtual functions
//============================================================================================
class WinBase{
protected:
	const WinDispatcher& dispatcher;
	HWND hWnd = nullptr;

public:
	WinBase(const WinDispatcher& dispatcher = WinDispatcher::sharedDispatcher());
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
	virtual void preRegisterClass(WNDCLASSEXA& wc) = 0;
	virtual void preCreateWindow(CREATESTRUCTA& cs) = 0;

	virtual bool onCreate(CREATESTRUCT* pcs);
	virtual bool onDestroy();
	virtual void onPaint();
};

//============================================================================================
// Simple window representaion inherited from WinBase
//============================================================================================
template <auto& class_name, LONG style = WS_POPUP>
class SimpleWindow : public WinBase{
protected:
	static ATOM class_atom;

public:
	SimpleWindow(const WinDispatcher& dispatcher = WinDispatcher::sharedDispatcher()) : WinBase(dispatcher){}
	virtual ~SimpleWindow() = default;

protected:
	virtual bool isRegisterd() override {return class_atom;};
	virtual void noticeRegisterd(ATOM atom) override {class_atom = atom;};
	virtual void preRegisterClass(WNDCLASSEXA& wc) override {wc.lpszClassName = class_name;};
	virtual void preCreateWindow(CREATESTRUCTA& cs) override{
		cs.style = style;
		cs.lpszClass = class_name;
	};
};

template <auto& class_name, LONG style>
ATOM SimpleWindow<class_name, style>::class_atom = 0;