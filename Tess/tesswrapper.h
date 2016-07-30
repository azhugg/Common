//
// 2016-07-27, jjuiddong
//
// tesseract ���� Ŭ����
// tesseract�� �״�� ����ϸ�, ������ ������ ����, ���� ������Ʈ�� �������.
//

#pragma once

#include <opencv2/highgui/highgui.hpp>


namespace tesseract {
	class TessBaseAPI;
}


namespace tess
{

	class cTessWrapper
	{
	public:
		cTessWrapper();
		virtual ~cTessWrapper();
		bool Init(const string &dataPath, const string &language);
		string Recognize(cv::Mat &img);


	public:
		tesseract::TessBaseAPI *m_tessApi;
	};

}