//
// 2016-05-28, jjuiddong
// templatematch ���
// ������� ���� ���� ������ ȭ�鿡 ����Ѵ�.
//
#pragma once


namespace cvproc
{

	class cTemplateMatch
	{
	public:
		cTemplateMatch(const float threshold=0.7f, const bool gray=true, const bool debug=false);
		virtual ~cTemplateMatch();
		bool Match(const cv::Mat &img, const cv::Mat &templ, 
			const string &comment="", OUT cv::Mat *out=NULL);
		bool Match(const string &srcFileName, const string &templFileName, 
			const string &option="", const string &comment = "");

	
	public:
		cv::Point m_leftTop;
		double m_min;
		double m_max;
		cv::Mat m_matResult;
		float m_threshold;
		bool m_gray;
		bool m_debug; // true�̸� ��Ī ����� ȭ�鿡 ����Ѵ�.
	};

}
