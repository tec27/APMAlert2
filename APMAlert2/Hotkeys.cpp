#include "stdafx.h"
#include "APMAlert2.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_HOTKEY:
			if(wParam == 0) toggleAlert();
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HotkeyThread::HotkeyThread() {}

void HotkeyThread::Setup() {
}

void HotkeyThread::Execute(void * data) {
	WNDCLASSEX windowclass;
    windowclass.cbSize        = sizeof(WNDCLASSEX);
    windowclass.style         = 0;
    windowclass.lpfnWndProc   = WndProc;
    windowclass.cbClsExtra    = 0;
    windowclass.cbWndExtra    = 0;
    windowclass.hInstance     = hInstance;
    windowclass.hIcon         = 0;
    windowclass.hCursor       = 0;
    windowclass.hbrBackground = 0;
    windowclass.lpszMenuName  = 0;
    windowclass.lpszClassName = "APMAlertMsgWindow";
    windowclass.hIconSm       = 0;

	if(!RegisterClassEx(&windowclass)) {
		ErrorMsg("RegisterClass");
		return;
	}

	HWND hwnd;
	hwnd = CreateWindowA("APMAlertMsgWindow", "APMAlert2", 0, 0, 0, 27, 27, HWND_MESSAGE, NULL, hInstance, NULL);
	if(hwnd == NULL) {
		ErrorMsg("CreateWindow/Msg");
		return;
	}

	UINT modifiers = 0;
	if((apmOptions->alertKey & 0x40000000) > 0) modifiers |= MOD_CONTROL;
	if((apmOptions->alertKey & 0x20000000) > 0) modifiers |= MOD_ALT;
	if((apmOptions->alertKey & 0x10000000) > 0) modifiers |= MOD_SHIFT;
	DWORD vkCode = apmOptions->alertKey & 0x0FFFFFFF;
	if(!RegisterHotKey(hwnd, 0, modifiers, vkCode)) {
		ErrorMsg("RegisterHotKey");
	}

	logInfo("Hotkeys registered successfully, hotkey message window running.");

	MSG msg;
	while(!isTerminated() && GetMessage(&msg, NULL, 0, 0))      //message loop
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hwnd);
}