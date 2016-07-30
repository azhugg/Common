//
//
// ARToolkit���� �ʿ��� ��ƿ��Ƽ �Լ��� ��Ҵ�.
//
//
#pragma once


namespace ar
{
	
	struct sQRPos
	{
		bool used;
		cv::Point pos;
	};

	void D3DConvMatrixView(float* src, D3DXMATRIXA16* dest);
	void D3DConvMatrixProjection(float* src, D3DXMATRIXA16* dest);

}

