
#pragma once


namespace framework
{

	// ������ ����
	HWND InitWindow(HINSTANCE hInstance, 
		const wstring &windowName, 
		const sRectf &windowRect,
		int nCmdShow,
		WNDPROC WndProc,
		const bool dualMonitor = true
	);

}
