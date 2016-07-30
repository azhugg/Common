//
// 2016-06-02, jjuiddong
// ���� �̵��ϴ� ����� �Ѵ�.
// �� �̵���, ESC, Enter Ű�� ���� �̵� �Ѵ�.
// cSceneTraverse �� �� �̵��� ���� Ű�� ��ȯ�Ѵ�.
//
#pragma once


namespace cvproc {
	namespace imagematch {

		class cSceneTraverse
		{
		public:
			cSceneTraverse();
			virtual ~cSceneTraverse();

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


			bool Init(const string &imageMatchScriptFileName, 
				const string &flowScriptFileName, 
				const int screenCaptureKey=VK_SNAPSHOT);
			STATE Update(const float deltaSeconds, const cv::Mat &img, OUT int &key);
			void Move(const string &sceneName);
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
			STATE OnProc(const cv::Mat &img, OUT int &key);
			STATE OnMenu(const cv::Mat &img, OUT int &key);
			STATE OnMenuDetect(const cv::Mat &img, OUT int &key);

			bool TreeMatch(cGraphScript::sNode *current, const cv::Mat &img, OUT sTreeMatchData &out, 
				const int loopCnt=-1);
			bool CheckMenuUpandDown(const cGraphScript::sNode *current, const cGraphScript::sNode *next);
			int GetNextMenuCount(const cGraphScript::sNode *current, const cGraphScript::sNode *next);
			int CheckNextMenuIndex(const cGraphScript::sNode *current, const cGraphScript::sNode *next);
			int CheckNextMenuIndex(const cGraphScript::sNode *current, const string &selectMenuId, 
				const cGraphScript::sNode *next);
			void Delay(const float delaySeconds, const STATE nextState);
			string GenerateInputID();
			cMatchResult& GetMatchResult();


		public:
			STATE m_state; // ���� ����
			bool m_isLog; // �α� ���
			cMatchScript2 m_matchScript;
			cGraphScript m_flowScript;
			cGraphScript::sNode *m_currentNode;
			cGraphScript::sNode *m_nextNode;
			vector<cGraphScript::sNode*> m_path; // �̵� ���
			string m_moveTo;	// �̵��� �� �̸�
			int m_tryMachingCount;
			int m_screenCaptureKey; // virtual key, VK_SNAPSHOT or VK_F11
			int m_genId; // default = 100000

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

} }