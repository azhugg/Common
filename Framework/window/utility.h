
#pragma once


namespace framework
{

	// 윈도우 생성
	HWND InitWindow(HINSTANCE hInstance, 
		const wstring &windowName, 
		const sRecti &windowRect,
		int nCmdShow,
		WNDPROC WndProc,
		const bool dualMonitor = true
	);

}
