#include "Window/BProcess.h"

BOOL CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam)
{
	PROCESS_INFO* pProcessWindow = (PROCESS_INFO*)lParam;

	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);

	if (pProcessWindow->dwProcessId == dwProcessId && IsWindowEnabled(hWnd) && GetParent(hWnd) == NULL)
	{
		pProcessWindow->hwndWindow = hWnd;
		return FALSE;
	}
	return TRUE;
}

HWND BProcess::BWCreateProcess()
{
	BCloseProcess();

	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = true;
	BOOL bRet = ::CreateProcess(NULL, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	if (bRet)
	{
		procwin.dwProcessId = pi.dwProcessId;
		HWND hwndRet = NULL;

		WaitForInputIdle(pi.hProcess, 5000);

		EnumWindows(EnumWindowCallBack, (LPARAM)&procwin);
		if (NULL == procwin.hwndWindow)
		{
			Sleep(200);
			EnumWindows(EnumWindowCallBack, (LPARAM)&procwin);
		}

		return procwin.hwndWindow;

	}
	return NULL;
}


void BProcess::BCloseProcess()
{
	if (pi.hProcess != NULL)
	{
		//关闭子进程的主线程句柄  
		CloseHandle(pi.hThread);

		//等待子进程的退出  
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD dwExitCode;
		//获取子进程的退出码  
		GetExitCodeProcess(pi.hProcess, &dwExitCode);

		//关闭子进程句柄  
		CloseHandle(pi.hProcess);
	}
}