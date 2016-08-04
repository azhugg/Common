//
// 2016-08-03, jjuiddong
// �޴��� �̵��ϸ鼭, �����ϴ� ����� ��.
//
// menu setting command
// category1 sub_category1 item1
// category2 sub_category2 item2
//
#pragma once


namespace cvproc {
	namespace imagematch {

		class cMenuControl
		{
		public:
			cMenuControl();
			virtual ~cMenuControl();

			enum STATE {
				WAIT, // ��� ��
				DELAY, // �����ð� ���� ����
				REACH, // ������ ����
				CAPTURE, // ���� ĸ�� ��û, ĸ�� ��, �׻� PROC ���°� �ȴ�.
				PROC, // ������ �Է¹޾�, �������� �̵��ϱ� ���� key �� ���
				MENU_MOVE, // �޴� �̵� ��. �����¿�
				MENU_DETECT,	// �޴� Ȯ�� �� �̵�
				ERR, // ���� ��, ���� �߻� 
			};

			bool Init(
				cMatchScript2 *matchScript, 
				const string &menuControlScriptFileName, 
				const int screenCaptureKey = VK_SNAPSHOT);

			STATE Update(const float deltaSeconds, const cv::Mat &img, OUT int &key);
			bool StartMenuSetting(const string &menuHeadName, const string &cmd);
			void Cancel();


		protected:
			STATE OnDelay(const float deltaSeconds, const cv::Mat &img, OUT int &key);
			STATE OnCapture(const cv::Mat &img, OUT int &key);
			STATE OnProc(const cv::Mat &img, OUT int &key);
			STATE OnMenu(const cv::Mat &img, OUT int &key);
			STATE OnMenuDetect(const cv::Mat &img, OUT int &key);


		public:
			STATE m_state; // ���� ����
			bool m_isLog; // �α� ���
			cMatchScript2 *m_matchScript;
			cGraphScript m_menuScript;
			cGraphScript::sNode *m_currentNode;
			string m_headName;
			int m_screenCaptureKey; // virtual key, VK_SNAPSHOT or VK_F11

			// menu setting command
			int m_currentCmdIdx;
			vector<string> m_cmds;

			// menu move
			int m_nextMenuIdx;
			int m_currentMenuIdx;

			// delay
			STATE m_nextState; // �����̰� ���� ��, ���� 
			bool m_isInitDelay; // ���������� ���� �ĺ��� ������ ����� �����ϱ� ���� ����
			float m_delayTime;
			float m_initDelayTime;

			// matchresult buffer
			int m_matchResultBuffIdx;
			cMatchResult m_matchResult[16];
		};

	}
}