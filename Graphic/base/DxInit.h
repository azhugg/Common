#pragma once

namespace graphic
{

	// ���̷�Ʈ X �ʱ�ȭ.
	bool InitDirectX(HWND hWnd, const int width, const int height, const UINT adapter,
		OUT D3DPRESENT_PARAMETERS &param, OUT LPDIRECT3DDEVICE9 &pDevice);


}
