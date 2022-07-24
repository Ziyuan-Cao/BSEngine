#pragma once 
#include <windows.h>

typedef struct
{
	HWND hwndWindow;
	DWORD dwProcessId;
}PROCESS_INFO;

class BProcess
{
public:
	HWND GetHWND() { return procwin.hwndWindow; };

	HWND BWCreateProcess();

	void BCloseProcess();

private:


	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	PROCESS_INFO procwin;
};