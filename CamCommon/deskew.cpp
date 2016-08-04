
#include "stdafx.h"
#include "deskew.h"

using namespace cvproc;
using namespace cv;


cDeSkew::cDeSkew() 
{
}

cDeSkew::~cDeSkew()
{
}


//
// canny�� ������ �����, ���� �� ������ ã��, 
// �� ���򼱿� ������ �� ��° ������ ã��, �� ���� ��������
//  ��ü ������ ȸ����Ű��, �μ����� ������� �ڽ��� �����ϰ�,  �����Ѵ�.
// ȸ���� ������ src�� ����ǰ�, �ڽ��� m_tessImg�� ����ȴ�.
//
bool cDeSkew::DeSkew( 
	INOUT Mat &src,
	const double arcAlpha, //= 0.005f,
	const int deSkewDebug, // = 0
	const bool isTesseractOcr) // = false
{
	Mat &dst = src;
	Mat tmp = dst.clone();

	if (dst.data && (dst.channels() >= 3))
		cvtColor(dst, dst, CV_BGR2GRAY);
	threshold(dst, dst, 20, 255, CV_THRESH_BINARY_INV);
	cv::Canny(dst, dst, 0, 100, 5);

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

	if (maxLenIdx == -1)
		return false; // error occur

	// �¿�� ���� �� �ٰ����� ������ ������ ����ؼ�, ���� �̹����� ������ �ǰ� ȸ����Ų��.
	cv::approxPolyDP(cv::Mat(contours[maxLenIdx]), approx, cv::arcLength(cv::Mat(contours[maxLenIdx]), true)*arcAlpha, true);

	double maxV = 0, maxV2 = 0;
	uint idx1 = 0, idx2 = 0;
	// ���� �� ������ ���Ѵ�. ������ circular line �� ����
	for (u_int i = 0; i < approx.size() - 1; ++i)
	{
		const int i2 = (i + 1) % approx.size(); // circular rect
		const double n = cv::norm(approx[i] - approx[i2]);
		if (maxV < n)
		{
			maxV = n;
			idx1 = i;
		}
	}

	// �� ��°�� �� ������ ���Ѵ�. ���� �� ���ΰ� �����̾�� �Ѵ�.

	// ���� �� ������ ���͸� ���Ѵ�.
	vector<Vector3> lineDirs(approx.size());
	for (u_int i = 0; i < approx.size(); ++i)
	{
		const int i2 = (i + 1) % approx.size(); // circular rect
		const Vector3 p1 = Vector3((float)approx[i].x, (float)approx[i].y, 0);
		const Vector3 p2 = Vector3((float)approx[i2].x, (float)approx[i2].y, 0);
		Vector3 v = p2 - p1;
		v.Normalize();
		lineDirs[i] = v;
	}

	struct sLine
	{
		double angle;
		uint i1; // approx index1
		uint i2; // approx index1
		double len; // length
	};
	//typedef pair<double, int> LineType; // angle, index
	vector<sLine> lines;
	lines.reserve(approx.size());

	// ���� �پ��ִ� ������ ���, �����Ѵ�. (y  ������ �Ǵ��Ѵ�.)
	const int minimumLength = 20;
	for (uint i = 0; i < approx.size(); ++i)
	{
		if (i == idx1)
			continue; // max �� ����

		const uint i2 = (i + 1) % approx.size(); // circular rect
		const double len = cv::norm(approx[i] - approx[i2]);
		const float angle = abs(lineDirs[idx1].DotProduct(lineDirs[i]));
		if (angle > 0.8f)
			lines.push_back({ angle, i, i2, len });
	}

	// ���� �� ���� �߰�
	lines.push_back({ 1, idx1, idx1 + 1, maxV });

	// �� ���γ���, �����̸鼭, �Ÿ��� �� ���� ���Ѵ�.
	struct sLinePair {
		float angle;
		int idx1;
		int idx2;
		double len;
	};

	vector<sLinePair> linePairs;
	linePairs.reserve(lines.size() * lines.size());

	for (uint i = 0; i < lines.size() - 1; ++i)
	{
		for (uint k = i + 1; k < lines.size(); ++k)
		{
			// y ���̰� ����ũ�� �̻��̾�� �Ѵ�.
			const int i1 = lines[i].i1;
			const int i12 = lines[i].i2;
			const int i2 = lines[k].i1;
			const int i22 = lines[k].i2;
			const int left1 = (approx[i1].x < approx[i12].x) ? i1 : i12;
			const int left2 = (approx[i2].x < approx[i22].x) ? i2 : i22;
			if (abs(approx[left1].y - approx[left2].y) < minimumLength)
				continue;

			sLinePair type;
			type.angle = abs(lineDirs[lines[i].i1].DotProduct(lineDirs[lines[k].i1]));
			type.idx1 = lines[i].i1;
			type.idx2 = lines[k].i1;
			type.len = lines[i].len + lines[k].len;
			linePairs.push_back(type);
		}
	}

	// ��鼭, ���� ������ �� ������ ���Ѵ�.
	std::sort(linePairs.begin(), linePairs.end(),
		[](const sLinePair &a, sLinePair &b)
	{
		return a.angle*a.len > b.angle*b.len;
	});

	if (!linePairs.empty())
	{
		idx1 = linePairs.front().idx1;
		idx2 = linePairs.front().idx2;
	}


	const int i1 = idx1;
	const int i12 = (idx1 + 1) % approx.size();
	const int i2 = idx2;
	const int i22 = (idx2 + 1) % approx.size();

	// debug display
	if (deSkewDebug)
	{
		cvtColor(dst, dst, CV_GRAY2BGR);
		for (u_int i = 0; i < approx.size() - 1; ++i)
			cv::line(dst, approx[i], approx[i + 1], Scalar(255, 255, 255), 2);
		cv::line(dst, approx[approx.size() - 1], approx[0], Scalar(255, 255, 255), 2);
		cv::line(dst, approx[i1], approx[i12], Scalar(255, 0, 0), 3);
		cv::line(dst, approx[i2], approx[i22], Scalar(0, 0, 255), 3);
	}

	//
	// p1 -------- p2
	// |                 |
	// |                 |
	// p3 -------- p4
	//
	const Vector3 p1 = (approx[i1].x < approx[i12].x) ? Vector3((float)approx[i1].x, (float)approx[i1].y, 0) : Vector3((float)approx[i12].x, (float)approx[i12].y, 0);
	const Vector3 p2 = (approx[i1].x > approx[i12].x) ? Vector3((float)approx[i1].x, (float)approx[i1].y, 0) : Vector3((float)approx[i12].x, (float)approx[i12].y, 0);
	const Vector3 p3 = (approx[i2].x < approx[i22].x) ? Vector3((float)approx[i2].x, (float)approx[i2].y, 0) : Vector3((float)approx[i22].x, (float)approx[i22].y, 0);
	const Vector3 p4 = (approx[i2].x > approx[i22].x) ? Vector3((float)approx[i2].x, (float)approx[i2].y, 0) : Vector3((float)approx[i22].x, (float)approx[i22].y, 0);
	Vector3 v = p2 - p1;
	v.Normalize();
	double angle = RAD2ANGLE(acos(v.DotProduct(Vector3(1, 0, 0))));
	if (v.y < 0)
		angle = -angle;

	m_pts[0] = Point((int)p1.x, (int)p1.y);
	m_pts[1] = Point((int)p2.x, (int)p2.y);
	m_pts[2] = Point((int)p4.x, (int)p4.y);
	m_pts[3] = Point((int)p3.x, (int)p3.y);

	m_deSkewPoint1 = Point((int)p1.x, (int)p1.y);
	m_deSkewPoint2 = Point((int)p3.x, (int)p3.y);
	m_deSkewPoint3 = (p2.x > p4.x) ? Point((int)p2.x, (int)p2.y) : Point((int)p4.x, (int)p4.y);
	const Mat affine_matrix = getRotationMatrix2D(Point((int)p1.x, (int)p1.y), angle, 1);

	if (deSkewDebug == 0)
		warpAffine(tmp, dst, affine_matrix, dst.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar::all(255));

	if (isTesseractOcr)
	{
		const int left = MAX(MIN(m_deSkewPoint1.x, m_deSkewPoint3.x) - 10, 0);
		const int right = MIN(MAX(m_deSkewPoint1.x, m_deSkewPoint3.x) + 10, dst.cols);
		m_tessImg = dst(Rect(left, MIN(m_deSkewPoint1.y, m_deSkewPoint2.y) - 5,
			right - left, (int)abs(m_deSkewPoint1.y - m_deSkewPoint2.y) + 10));
	}

	return true;
}
