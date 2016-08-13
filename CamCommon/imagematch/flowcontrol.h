//
// 2016-06-02, jjuiddong
// ���� �̵��ϴ� ����� �Ѵ�.
// �� �̵���, ESC, Enter Ű�� ���� �̵� �Ѵ�.
// cFlowControl �� �� �̵��� ���� Ű�� ��ȯ�Ѵ�.
//
#pragma once


namespace cvproc {
	namespace imagematch {

		class cFlowControl
		{
		public:
			cFlowControl();
			virtual ~cFlowControl();

			enum STATE {
				WAIT, // ��� ��
				DELAY, // �����ð� ���� ����
				REACH, // ������ ����
				CAPTURE, // ���� ĸ�� ��û, ĸ�� ��, �׻� PROC ���°� �ȴ�.
				CAPTURE_ERR, // ���� ĸ�� ��û, �νĿ� �����ؼ�, �� ����Ѵ�.
				CAPTURE_NEXT, // ���� ĸ�� ��û, ĸó ��, ���� ������ �Ѿ��.
				CAPTURE_MENU, // ���� ĸ�� ��û, ĸó ��, ���� �޴� ���¸� Ȯ���ϰ�, ���� �ܰ�� �Ѿ��.
				PROC, // ������ �Է¹޾�, ���� ��ġ�� �ľ��� ��, �������� �̵��ϱ� ���� key �� ���
				NEXT, // ������ �Է¹޾�,  ���� ���� �޴��� �Ѿ��. ���� ��ġ�� �ľ����� �ʴ´�.
				MENU_MOVE, // �޴� �̵� ��. �����¿�
				MENU_DETECT,	// �޴� Ȯ�� �� �̵�
				ERR, // ���� ��, ���� �߻� 
			};


			bool Init(const string &imageMatchScriptFileName, 
				const string &flowScriptFileName, 
				const int screenCaptureKey=VK_SNAPSHOT);
			STATE Update(const float deltaSeconds, const cv::Mat &img, OUT int &key);
			bool Command(const string &cmdFileName);
			void Cancel();
			

		protected:
			struct sTreeMatchData
			{
				cGraphScript::sNode *node;
				string result;
				int loopCnt;
			};

			STATE OnDelay(const float deltaSeconds, const cv::Mat &img, OUT int &key);
			STATE OnCapture(const cv::Mat &img, OUT int &key);
			STATE OnCaptureNext(const cv::Mat &img, OUT int &key);
			STATE OnCaptureMenu(const cv::Mat &img, OUT int &key);
			STATE OnProc(const cv::Mat &img, OUT int &key);
			STATE OnMenu(const cv::Mat &img, OUT int &key);
			STATE OnMenuDetect(const cv::Mat &img, OUT int &key);
			STATE OnNext(const cv::Mat &img, OUT int &key);
			STATE NextStep(const cv::Mat &img, cGraphScript::sNode *detectNode, OUT int &key);

			bool TreeMatch(cGraphScript::sNode *current, const cv::Mat &img, OUT sTreeMatchData &out, 
				const int loopCnt=-1);
			int CheckMenuUpandDown(const cGraphScript::sNode *current, const cGraphScript::sNode *next);
			int GetNextMenuCount(const cGraphScript::sNode *current, const cGraphScript::sNode *next);
			int CheckNextMenuIndex(const cGraphScript::sNode *current, const cGraphScript::sNode *next);
			int CheckNextMenuIndex(const cGraphScript::sNode *current, const string &selectMenuId, 
				const cGraphScript::sNode *next);
			STATE Delay(const float delaySeconds, const STATE nextState);
			string GenerateInputID();
			cMatchResult& GetMatchResult();
			string GetStateString(const STATE state);
			STATE NextCommand();


		public:
			STATE m_state; // ���� ����
			bool m_isLog; // �α� ���
			cMatchScript2 m_matchScript;
			cGraphScript m_flowScript;
			cGraphScript::sNode *m_detectNode;
			cGraphScript::sNode *m_nextNode;
			vector<cGraphScript::sNode*> m_path; // �̵� ���
			vector<string> m_commands;
			int m_cmdIdx; // �������� m_commands Index
			int m_tryMachingCount;
			int m_screenCaptureKey; // virtual key, VK_SNAPSHOT or VK_F11
			int m_genId; // default = 100000
			cv::Mat m_lastImage;

			// menu move
			int m_nextMenuIdx;
			int m_currentMenuIdx;
			bool m_isMenuCheck; // �������� �Ѿ�� ���� �޴��� �ѹ� Ȯ���Ѵ�.

			// delay
			STATE m_nextState; // �����̰� ���� ��, ���� 
			bool m_isInitDelay; // ���������� ���� �ĺ��� ������ ����� �����ϱ� ���� ����
			float m_delayTime;
			float m_initDelayTime;

			// matchresult buffer
			int m_matchResultBuffIdx;
			cMatchResult m_matchResult[16];
		};

} }