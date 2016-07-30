//
// 2016-07-17, jjuiddong
// Fraps ���� ĸ���� �̹����� �ε��Ѵ�.
// 
// �����带 ���鼭, Fraps ���丮�� �˻��� ������ �����Ǹ� �ε��Ѵ�.
// �ε��� �̹����� �����Ѵ�.
//
#pragma once


namespace cvproc
{
	class cFrapsCapture
	{
	public:
		cFrapsCapture();
		virtual ~cFrapsCapture();

		bool Init(const string &frapsScreenShotPath, const int key);
		void ScreenShot();
		bool IsUpdateScreenShot();
		void Clear();


	public:
		// ������ ���� �Լ���
		void KeyDown();
		bool SearchFrapsDir();


	public:
		enum STATE {
			WAIT,  // �����
			SCREENSHOT,	// ��ũ���� ��û
			KEYDOWN, // ��ũ���� Ű �ٿ�
			SEARCH_DIR, // Fraps ���丮 �˻�
			END, // ��ũ���� �̹��� �������� ����.
		};
		
		STATE m_state;
		string m_frapsScreenShotPath;
		cv::Mat m_img; // ��ũ���� �̹���
		int m_key; // Fraps ��ũ���� Ű
		int m_keyDownDelay; // default : 100 milliseconds
		list<string> m_exts; // *.bmp

		// ������ ����
		std::thread m_thread;
		bool m_isRunThread;
	};
}
