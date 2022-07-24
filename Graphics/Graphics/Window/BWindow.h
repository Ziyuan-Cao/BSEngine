#pragma once

#include <windows.h>
#include <WindowsX.h>
#include <string>

//#include "Tool/TTimer.h"
#include "Pre_Define.h"

//
// 1.构建窗体
// 2.消息函数重载，对各种进行操作
class ENGINEDLL_API BWindow
{
public:
	bool InitMainWindow(HINSTANCE IHINSTANCE, UINT IWidth, UINT IHeight);

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static BWindow* GetInstance();

	HWND GetHWND() { return Mainwindowhandle; };

	int GetWidth() { return Clientwidth; };

	int GetHeight() { return Clientheight; };

	void SetConctrolFunction(void (*IOnMouseDown_ptr)(WPARAM, int, int),
		void (*IOnMouseUp_ptr)(WPARAM, int, int),
		void (*IOnMouseMove_ptr)(WPARAM, int, int))
	{
		InputFunction_OnMouseDown_ptr = IOnMouseDown_ptr;
		InputFunction_OnMouseUp_ptr = IOnMouseUp_ptr;
		InputFunction_OnMouseMove_ptr = IOnMouseMove_ptr;
	}

private:

	static BWindow* Windowinstance;

	std::wstring Mainwindowcaption = L"Main Window";
	HINSTANCE Instancehandle = nullptr; // application instance handle
	HWND      Mainwindowhandle = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

	//TTimer Timer;

	int Clientwidth = 800;
	int Clientheight = 600;

	void (*InputFunction_OnMouseDown_ptr)(WPARAM, int, int);
	void (*InputFunction_OnMouseUp_ptr)(WPARAM, int, int);
	void (*InputFunction_OnMouseMove_ptr)(WPARAM, int, int);

};