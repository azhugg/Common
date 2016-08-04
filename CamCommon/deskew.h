//
// 2016-08-01, jjuiddong
// deSkew ���
// dirt3 menu deskew�� ���� ���
//
#pragma once


namespace cvproc
{

	class cDeSkew
	{
	public:
		cDeSkew();
		virtual ~cDeSkew();

		bool DeSkew(INOUT cv::Mat &src, const double arcAlpha=0.005f, 
			const int deSkewDebug=0, const bool isTesseractOcr=false);


	public:
		cv::Mat m_tessImg;

		// skew ��ɾ�� ���õ� skew ��ġ 
		//
		// deSkewPoint1 ------------------- m_deSkewPoint3
		// |                                                    |
		// |                                                    |
		// deSkewPoint2 ------------------- *
		cv::Point m_deSkewPoint1;
		cv::Point m_deSkewPoint2;
		cv::Point m_deSkewPoint3;

		// p1 -------- p2
		// |                 |
		// |                 |
		// p4 -------- p3
		cv::Point m_pts[4];
	};

}
