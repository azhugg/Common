
#include "stdafx.h"
#include "simplematchscript.h"
#include "matchprocessor.h"


using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;


cSimpleMatchScript::cSimpleMatchScript(cMatchScript2 *matchScript)
	: m_matchScript(matchScript)
	, m_state(WAIT)
	, m_curIdx(0)
{
	const int maxLen = sizeof(m_matchResult) / sizeof(cMatchResult);
	for (int i = 0; i < maxLen; ++i)
		m_matchResult[i].Clear();

	m_tess.Init("./", "eng", "dictionary.txt");
}

cSimpleMatchScript::~cSimpleMatchScript()
{
}


//		- roi=x,y,width,height
//     - bgr=num1,num2,num3
//         - Mat &= Scalar(num1,num2,num3)
//     - scale=num
//         - Mat *= num
//     - gray=0/1
//         - gray convert
//     - hsv=num1,num2,num3,num4,num5,num6
//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
// - templatematch = filename
//		- @label_select
// - featurematch = filename
//		- @label_select
// - imagematch = labelname
//		- @tree_label
// - img=@capture_select
string cSimpleMatchScript::Match(INOUT cv::Mat &src, OUT cv::Mat &dst, const string &script
	,const string &label_select, const string &capture_select, const string &tree_label) // label_select="", tree_label=""
{
	AutoCSLock cs(m_processCS);

	m_src = src.clone(); // ��� ������ ����� ��, ����
	dst = src.clone();
	m_tessImg = Mat();

	vector<Mat> accMat;
	accMat.reserve(8);

	// ����, ���๮�� �������� �и�
	vector<string> argv0;
	common::tokenizer(script, " ", "", argv0);
	vector<string> argv;
	for each (auto arg in argv0)
		common::tokenizer(arg, "\r\n", "", argv);

	const int t1 = timeGetTime();

	string errMsg;
	char value[128];
	char value2[128];
	cDeSkew deSkew;
	const u_int argc = argv.size();
	for (u_int i = 0; i < argc; ++i)
	{
		trim(argv[i]);

		//----------------------------------------------------------------------
		// roi=x,y,width,height
		cv::Rect roi = { 0,0,0,0 };
		sscanf(argv[i].c_str(), "roi=%d,%d,%d,%d", &roi.x, &roi.y, &roi.width, &roi.height);
		if (roi.area() > 0)
		{
			if (dst.data)
				dst = dst(roi);
			continue;
		}

		//----------------------------------------------------------------------
		// rect=x,y,width,height
		cv::Rect rect = { 0,0,0,0 };
		sscanf(argv[i].c_str(), "rect=%d,%d,%d,%d", &rect.x, &rect.y, &rect.width, &rect.height);
		if (rect.area() > 0)
		{
			cRectContour r(rect);
			r.Draw(dst, Scalar(0, 0, 255), 2);
			continue;
		}

		//----------------------------------------------------------------------
		//     - bgr=num1,num2,num3
		//         - Mat &= Scalar(num1,num2,num3)
		int bgr[3] = { 0,0,0 };
		sscanf(argv[i].c_str(), "bgr=%d,%d,%d", bgr, bgr + 1, bgr + 2);
		if ((bgr[0] != 0) || (bgr[1] != 0) || (bgr[2] != 0))
		{
			if (dst.data)
				dst &= Scalar(bgr[0], bgr[1], bgr[2]);
			continue;
		}


		//----------------------------------------------------------------------
		//     - scale=num
		//         - Mat *= num
		float scale = 0;
		sscanf(argv[i].c_str(), "scale=%f", &scale);
		if (scale != 0)
		{
			if (dst.data)
				dst *= scale;
			continue;
		}


		//----------------------------------------------------------------------
		//     - gray=0/1
		//         - gray convert
		int gray = 0;
		sscanf(argv[i].c_str(), "gray=%d", &gray);
		if (gray)
		{
			if (dst.data && (dst.channels() >= 3))
				cvtColor(dst, dst, CV_BGR2GRAY);
			continue;
		}

		//----------------------------------------------------------------------
		int  thresh1 = 0;
		sscanf(argv[i].c_str(), "threshold=%d", &thresh1);
		if (thresh1 > 0)
		{
			if (dst.data && (dst.channels() >= 3))
				cvtColor(dst, dst, CV_BGR2GRAY);
			threshold(dst, dst, thresh1, 255, CV_THRESH_BINARY);
			continue;
		}


		//----------------------------------------------------------------------
		double thresh_c = 0;
		int block_size = 0;
		sscanf(argv[i].c_str(), "adapthreshold=%d,%lf", &block_size, &thresh_c);
		if (thresh_c > 0)
		{
			if (dst.data && (dst.channels() >= 3))
				cvtColor(dst, dst, CV_BGR2GRAY);
			adaptiveThreshold(dst, dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, block_size, thresh_c);
			continue;
		}

		//----------------------------------------------------------------------
		int  thresh = 0;
		sscanf(argv[i].c_str(), "invert=%d", &thresh);
		if (thresh > 0)
		{
			if (dst.data && (dst.channels() >= 3))
				cvtColor(dst, dst, CV_BGR2GRAY);
			threshold(dst, dst, thresh, 255, CV_THRESH_BINARY_INV);
			continue;
		}


		//----------------------------------------------------------------------
		//     - hsv=num1,num2,num3,num4,num5,num6
		//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
		int hsv[6] = { 0,0,0, 0,0,0 }; // inrage
		sscanf(argv[i].c_str(), "hsv=%d,%d,%d,%d,%d,%d", hsv, hsv + 1, hsv + 2, hsv + 3, hsv + 4, hsv + 5);
		if ((hsv[0] != 0) || (hsv[1] != 0) || (hsv[2] != 0) || (hsv[3] != 0) || (hsv[4] != 0) || (hsv[5] != 0))
		{
			if (dst.data)
			{
				cvtColor(dst, dst, CV_BGR2HSV);
				inRange(dst, cv::Scalar(hsv[0], hsv[1], hsv[2]), cv::Scalar(hsv[3], hsv[4], hsv[5]), dst);
				cvtColor(dst, dst, CV_GRAY2BGR);
				accMat.push_back(dst.clone());
			}
			continue;
		}


		//----------------------------------------------------------------------
		//     - hls=num1,num2,num3,num4,num5,num6
		//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
		int hsl[6] = { 0,0,0, 0,0,0 }; // inrage
		sscanf(argv[i].c_str(), "hls=%d,%d,%d,%d,%d,%d", hsl, hsl + 1, hsl + 2, hsl + 3, hsl + 4, hsl + 5);
		if ((hsl[0] != 0) || (hsl[1] != 0) || (hsl[2] != 0) || (hsl[3] != 0) || (hsl[4] != 0) || (hsl[5] != 0))
		{
			if (dst.data)
			{
				cvtColor(dst, dst, CV_BGR2HLS);
				inRange(dst, cv::Scalar(hsl[0], hsl[1], hsl[2]), cv::Scalar(hsl[3], hsl[4], hsl[5]), dst);
				cvtColor(dst, dst, CV_GRAY2BGR);
				accMat.push_back(dst.clone());
			}
			continue;
		}

		if (argv[i] == "acc" && !accMat.empty())
		{
			if (!accMat.empty())
			{
				Mat tmp(accMat.front().rows, accMat.front().cols, accMat.front().flags);
				for each (auto &m in accMat)
					tmp += m;
				dst = tmp;
			}
			continue;
		}


		//----------------------------------------------------------------------
		// img=@capture_select
		ZeroMemory(value, sizeof(value));
		sscanf(argv[i].c_str(), "img=%s", value);
		if (value[0] != NULL)
		{
			dst = imread((string("@capture_select") == value) ? capture_select.c_str() : value);
			if (!dst.data)
			{
				errMsg = "Not Found Capture Image \n";
				return errMsg; // ���н� ����.
			}

			m_src = dst.clone();
			src = dst.clone();
		}


		//----------------------------------------------------------------------
		// hough
		ZeroMemory(value, sizeof(value));
		sscanf(argv[i].c_str(), "hough=%s", value);
		if (value[0] != NULL)
		{
			std::vector<cv::Vec2f> lines;
			cv::HoughLines(dst, lines, 1, 0.1f, 80);
			for each (auto line in lines)
			{
				float rho = line[0];   // ù ��° ��Ҵ� rho �Ÿ�
				float theta = line[1]; // �� ��° ��Ҵ� ��Ÿ ����

				const double PI = MATH_PI;
				if (theta < PI / 4.f || theta > 3.f*PI / 4.f) { // ���� ��
					cv::Point pt1((int)(rho / cos(theta)), 0); // ù �࿡�� �ش� ���� ������   
					cv::Point pt2((int)((rho - dst.rows*sin(theta)) / cos(theta)), dst.rows);
					// ������ �࿡�� �ش� ���� ������
					cv::line(dst, pt1, pt2, cv::Scalar(255), 1); // �Ͼ� ������ �׸���
				}
				else { // ���� ��
					cv::Point pt1(0, (int)(rho / sin(theta))); // ù ��° ������ �ش� ���� ������  
					cv::Point pt2(dst.cols, (int)((rho - dst.cols*cos(theta)) / sin(theta)));
					// ������ ������ �ش� ���� ������
					cv::line(dst, pt1, pt2, cv::Scalar(255), 1); // �Ͼ� ������ �׸���
				}
			}
		}


		//----------------------------------------------------------------------
		// canny=threshold
		ZeroMemory(value, sizeof(value));
		sscanf(argv[i].c_str(), "canny=%s", value);
		if (value[0] != NULL)
		{
			const int threshold1 = atoi(value);
			cv::Canny(dst, dst, 0, threshold1, 5);
		}


		//----------------------------------------------------------------------
		// findcontours=arcAlpha
		float arcAlpha = 0;
		sscanf(argv[i].c_str(), "findcontours=%f", &arcAlpha);
		if (arcAlpha != 0)
		{
			cRectContour rect;
			vector<vector<cv::Point>> contours;
			cv::findContours(dst, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

			// �¿�� ����� �ٰ����� ã�´�.
			int maxLenIdx = -1;
			int maxLength = 0;
			std::vector<cv::Point> maxLine;
			std::vector<cv::Point> approx;
			for (u_int i = 0; i < contours.size(); i++)
			{
				cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*arcAlpha, true);
				if (approx.empty())
					continue;

				int minX = 10000;
				int maxX = 0;
				for each (auto pos in approx)
				{
					if (minX > pos.x)
						minX = pos.x;
					if (maxX < pos.x)
						maxX = pos.x;
				}

				const int len = abs(maxX - minX);
				if (maxLength < len)
				{
					maxLength = len;
					maxLenIdx = i;
				}
			}

			if (maxLenIdx != -1)
			{
				cv::approxPolyDP(cv::Mat(contours[maxLenIdx]), approx, cv::arcLength(cv::Mat(contours[maxLenIdx]), true)*arcAlpha, true);

				double maxV = 0;
				int idx = 0;
				for (u_int i=0; i < approx.size()-1; ++i)
				{
					const double n = cv::norm(approx[i] - approx[i + 1]);
					if (maxV < n)
					{
						maxV = n;
						idx = i;
					}
				}

				cv::line(dst, approx[idx], approx[idx + 1], Scalar(255), 3);

				for (u_int i = 0; i < approx.size() - 1; ++i)
					cv::line(dst, approx[i], approx[i + 1], Scalar(255), 1);
				cv::line(dst, approx[approx.size() - 1], approx[0], Scalar(255), 1);
			}
		}


		//----------------------------------------------------------------------
		// deskew=arcAlpha
		float arcAlpha2 = 0;
		int deSkewDebug = 0;
		sscanf(argv[i].c_str(), "deskew=%f,%d", &arcAlpha2, &deSkewDebug);
		if (arcAlpha2 != 0)
		{
			deSkew.DeSkew(dst, arcAlpha2, deSkewDebug, true);
		}


		//----------------------------------------------------------------------
		// TemplateMatch
		ZeroMemory(value, sizeof(value));
		sscanf(argv[i].c_str(), "templatematch=%s", value);
		if (value[0] != NULL)
		{
			Mat label = imread((string("@label_select") == value)? label_select.c_str() : value, 
				IMREAD_GRAYSCALE);
			if (!label.data)
			{
				errMsg = "Not Found Label Image \n";
				break;
			}

			cvproc::cTemplateMatch match(0.7f, true, false);
			match.Match(dst, label, "Template Match", &dst);
			break; // ��ɾ ������ ��, �����Ѵ�.
		}

		//----------------------------------------------------------------------
		// FeatureMatch
		ZeroMemory(value, sizeof(value));
		sscanf(argv[i].c_str(), "featurematch=%s", value);
		if (value[0] != NULL)
		{
			Mat label = imread((string("@label_select") == value) ? label_select.c_str() : value,
				IMREAD_GRAYSCALE);
			if (!label.data)
			{
				errMsg = "Not Found Label Image \n";
				break;
			}

			cvproc::cFeatureMatch match(true, false);
			match.Match(dst, label, "Feature Match", &dst);
			break; // ��ɾ ������ ��, �����Ѵ�.
		}

		//----------------------------------------------------------------------
		// ImageMatch
		ZeroMemory(value, sizeof(value));
		ZeroMemory(value2, sizeof(value2));
		sscanf(argv[i].c_str(), "imagematch=%s", value);
		sscanf(argv[i].c_str(), "imagematch2=%s", value2);
		if ((value[0] != NULL) || (value2[0] != NULL))
		{
			char *valueTmp = (value[0] != NULL) ? value : value2;
			const string treeLabelName = (string("@tree_label") == valueTmp) ? tree_label : valueTmp;

			//m_src = src.clone(); // ��� ������ ����� ��, ����
			m_matchResult[m_curIdx].Init(m_matchScript, dst, treeLabelName, 0,
				(sParseTree*)m_matchScript->FindTreeLabel(treeLabelName), 
				true, false);

			m_matchResult[m_curIdx].m_traverseType = (value[0] != NULL) ? 0 : 1;

			// ������� �����Ѵ�.
			m_state = BEGIN_MATCH;
			cMatchProcessor::Get()->Match(m_matchResult[m_curIdx]);

			break; // ��ɾ ������ ��, �����Ѵ�.
		}

		//----------------------------------------------------------------------
		//     - dilate
		if (argv[i] == "dilate")
		{
			dilate(dst, dst, Mat());
			continue;
		}

		//----------------------------------------------------------------------
		//     - erode
		if (argv[i] == "erode")
		{
			erode(dst, dst, Mat());
			continue;
		}


		//----------------------------------------------------------------------
		// tess=dictionary file name
		ZeroMemory(value, sizeof(value));
		ZeroMemory(value2, sizeof(value2));
		sscanf(argv[i].c_str(), "tess=%s", value);
		sscanf(argv[i].c_str(), "tess=%s", value2);
		if ((value[0] != NULL) || (value2[0] != NULL))
		{
			// deskew �� ������ �̹����� ���� �ν��� �Ѵ�.
			string srcStr;
			string fastStr;
			string result;
			int t0, t01, t1, t2;
			float maxFitness;
			if (deSkew.m_tessImg.data)
			{
				m_tessImg = deSkew.m_tessImg;
				deSkew.m_tessImg = Mat();				
				t0 = timeGetTime();
				srcStr = m_tess.Recognize(m_tessImg);
				t01 = timeGetTime();
				result = m_tess.Dictionary2(value, srcStr, fastStr, maxFitness, t1, t2);
			}
			else
			{
				t0 = timeGetTime();
				srcStr = m_tess.Recognize(dst);
				t01 = timeGetTime();
				result = m_tess.Dictionary2(value, srcStr, fastStr, maxFitness, t1, t2);
			}

			if (dst.rows < 1080)
			{
				Mat tmp(Size(dst.cols, 1080), dst.flags);
				tmp.setTo(Scalar(255, 255, 255));
				dst.copyTo(tmp(Rect(0,0,dst.cols, dst.rows)));
				dst = tmp;
			}

			Scalar color(0, 0, 255);
			if (dst.channels() == 1)
				cvtColor(dst, dst, CV_GRAY2BGR);

			putText(dst, common::format("tess source = %s", srcStr.c_str()).c_str(),
				Point(0, 90), 1, 2.f, color, 2);

			putText(dst, common::format("tess FastSearch = %s", fastStr.c_str()).c_str(),
				Point(0, 120), 1, 2.f, color, 2);

			putText(dst, common::format("tess result = %s", result.c_str()).c_str(), 
				Point(0, 150), 1, 2.f, color, 2);

			putText(dst, common::format("tess recognition time = %d, t1 = %d, t2 = %d", t01-t0, t1, t2).c_str(),
				Point(0, 180), 1, 2.f, color, 2);
		}

	}

	const int t2 = timeGetTime();
	if (dst.data)
		putText(dst, common::format("time=%d", t2-t1).c_str(), Point(0, 60), 1, 2.f, Scalar(255, 255, 255), 2);

	return errMsg;
}


// return value : -1 = ��Ī�� ���� ���� ����
//							0 = ��Ī ��
//						    1 = ��Ī�� �Ϸ�� ����
int cSimpleMatchScript::IsMatchComplete()
{
	RETV(m_state == WAIT, -1);

	if (m_matchResult[m_curIdx].m_isEnd)
	{
		// ��Ī�� �Ϸ����, �ʱ�ȭ �ȴ�.
		m_state = WAIT;
		return 1;
	}

	return 0;
}


cMatchResult& cSimpleMatchScript::GetCurrentMatchResult()
{
	return m_matchResult[m_curIdx];
}


// ��������� �����Ѵ�.
bool cSimpleMatchScript::UpdateMatchResult(OUT cv::Mat &dst)
{
	AutoCSLock cs(m_processCS);

	dst = m_src.clone();

	// ��� ���� ���
	putText(dst, m_matchResult[m_curIdx].m_resultStr.c_str(), Point(0, 30), 1, 2, Scalar(0, 0, 255), 2);

	// ��Ī�� �̹����� dst�� ����Ѵ�.
	for (int i = 0; i < 1024; ++i)
	{
		if (m_matchResult[m_curIdx].m_data[i].result > 0)
		{
			// Matching Rect ���
			cRectContour rect;
			if (m_matchScript->m_nodeTable[i]->matchType == 0) // TemplateMatch
			{
				rect.Init(m_matchResult[m_curIdx].m_data[i].matchRect);
				rect.Draw(dst, Scalar(0, 0, 255), 2);
			}
			else // Feature Match
			{
				vector<cv::Point> pts(m_matchResult[m_curIdx].m_data[i].matchRect2, 
					m_matchResult[m_curIdx].m_data[i].matchRect2 + 4);
				rect.Init(pts);
				rect.Draw(dst, Scalar(0, 0, 255), 2);
			}

			// ��Ī�� �̹��� ��� ���
			putText(dst, m_matchScript->m_nodeTable[i]->name, rect.m_contours[0] + Point(0, 30), 1, 2.f, Scalar(0, 0, 255), 2);
			putText(dst, common::format("time=%d", timeGetTime() - m_matchResult[m_curIdx].m_beginTime).c_str(), Point(0, 60), 1, 2.f, Scalar(255, 255, 255), 2);
		}
	}

	NextIndex();

	return true;
}


// ��Ī ���� ����.
void cSimpleMatchScript::TerminiateMatch()
{
	m_matchResult[m_curIdx].TerminateMatch();
	NextIndex();
}


void cSimpleMatchScript::NextIndex()
{
	const int maxLen = sizeof(m_matchResult) / sizeof(cMatchResult);
	m_curIdx = (++m_curIdx) % maxLen;
}
