
#pragma once


namespace framework
{

	// ������ ����
	HWND InitWindow(HINSTANCE hInstance, 
		const wstring &windowName, 
		const sRecti &windowRect,
		int nCmdShow,
		WNDPROC WndProc,
		const bool dualMonitor = true
	);

}
