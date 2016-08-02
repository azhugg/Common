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

	class cDictionary;
	class cTessWrapper
	{
	public:
		cTessWrapper();
		virtual ~cTessWrapper();
		bool Init(const string &dataPath, const string &language, 
			const string &dictionaryFileName);
		string Recognize(cv::Mat &img);
		string Dictionary(const string &src, const int flags=0);
		string Dictionary2(const string &src, OUT string &out, OUT int &t1, OUT int &t2); // debugging


	public:
		tesseract::TessBaseAPI *m_tessApi;
		cDictionary *m_dict;
	};

}