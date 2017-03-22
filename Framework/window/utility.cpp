
#include "stdafx.h"
#include "utility.h"

using namespace framework;


HWND framework::InitWindow(HINSTANCE hInstance, const wstring &windowName, const RECT &windowRect, int nCmdShow, WNDPROC WndProc,
	const bool dualMonitor)
	// dualMonitor=false
{
	const int X = windowRect.left;
	const int Y = windowRect.top;
	const int WIDTH = windowRect.right - windowRect.left;
	const int HEIGHT = windowRect.bottom - windowRect.top;

	//������ Ŭ���� ���� ����
	//���� �̷��� ������ ����ڴ� ��� ����
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;			//�����쿡�� ����ϴ� ������ �޸𸮼���( �׳� 0 �̴�  �Ű澲������ )
	WndClass.cbWndExtra = 0;			//�����쿡�� ����ϴ� ������ �޸𸮼���( �׳� 0 �̴�  �Ű澲������ )
	WndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);		//������ ������
	WndClass.hCursor = LoadCursor( NULL, IDC_ARROW );			//�������� Ŀ����� ����
	WndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );		//����������ܸ�� ����
	WndClass.hInstance = hInstance;				//���α׷��ν��Ͻ��ڵ� 
	WndClass.lpfnWndProc = (WNDPROC)WndProc;			//������ ���ν��� �Լ� ������
	WndClass.lpszMenuName = NULL;						//�޴��̸� ������ NULL
	WndClass.lpszClassName = windowName.c_str();				//���� �ۼ��ϰ� �ִ� ������ Ŭ������ �̸�
	//WndClass.style	 = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;	//������ �׸��� ��� ���� ( ����� ����ɶ� ȭ�鰻�� CS_HREDRAW | CS_VREDRAW )
	WndClass.style = CS_HREDRAW | CS_VREDRAW;	//������ �׸��� ��� ���� ( ����� ����ɶ� ȭ�鰻�� CS_HREDRAW | CS_VREDRAW )

	//������ �ۼ��� ������ Ŭ�������� ���
	RegisterClass( &WndClass );

	//������ ����
	HWND hWnd = CreateWindow(
		windowName.c_str(),
		windowName.c_str(),
		WS_OVERLAPPEDWINDOW,
		X,
		Y,
		WIDTH,
		HEIGHT,
		GetDesktopWindow(),
		NULL,
		hInstance,
		NULL
		);

	//�����츦 ��Ȯ�� �۾����� ũ��� �����
	RECT rcClient = { 0, 0, WIDTH, HEIGHT};
	AdjustWindowRect( &rcClient, WS_OVERLAPPEDWINDOW, FALSE );	//rcClient ũ�⸦ �۾� �������� �� ������ ũ�⸦ rcClient �� ���ԵǾ� ���´�.

	const int width = rcClient.right - rcClient.left;
	const int height = rcClient.bottom - rcClient.top;
	const int screenCX = GetSystemMetrics(SM_CXSCREEN) * (dualMonitor? 2 : 1);
	const int screenCY = GetSystemMetrics(SM_CYSCREEN);
	const int x = screenCX/2 - width/2;
	int y = screenCY/2 - height/2;

	const float hRate = (float)height / (float)screenCY;
	if (hRate > 0.95f)
		y = 0;

	//������ ũ��� ������ ��ġ�� �ٲپ��ش�.
	SetWindowPos( hWnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE );
	MoveWindow(hWnd, x, y, width, height, TRUE);

	ShowWindow( hWnd, nCmdShow );
	
	return hWnd;
}
