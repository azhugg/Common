//
// 2016-03-17, jjuiddong
//
// QRCode�� ���ڹ��̷� ���� �Ǿ����� ��, ī�޶� ��ġ�� ���� ����� 4���� ã�´�.
//
//
#pragma once

#include "arutil.h"


namespace ar
{

	void Init();
	bool FindQuadrant(const int tableW, const int tableH, const int nearId, const ar::sQRPos *qrMap, OUT int *nearQuad);

}
