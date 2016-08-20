//
// 2016-07-11, jjuiddong
// �̹��� ��Ī ��ũ��Ʈ
// ���� ����
//
// - templatematch = filename
// - featurematch = filename
// - imagematch = filename
//
//	symbol
//		- @label_select : argument file 1
//     - @tree_label : argument file 2
//
#pragma once



namespace cvproc {
	namespace imagematch {

		class cSimpleMatchScript
		{
		public:
			cSimpleMatchScript(cMatchScript2 *matchScript);
			virtual ~cSimpleMatchScript();
			string Match(INOUT cv::Mat &src, OUT cv::Mat &dst, const string &script, 
				const string &label_select="", const string &capture_select="", const string &tree_label="");
			int IsMatchComplete();
			bool UpdateMatchResult(OUT cv::Mat &dst);
			void TerminiateMatch();
			cMatchResult& GetCurrentMatchResult();


		protected:
			void NextIndex();


		public:
			enum STATE { WAIT, BEGIN_MATCH};
			STATE m_state;
			cMatchScript2 *m_matchScript;
			cMatchResult m_matchResult[32];
			cv::Mat m_src;
			cv::Mat m_tessImg;
			int m_curIdx;
			int m_beginMatchTime;
			tess::cTessWrapper m_tess;
			CriticalSection m_processCS;
		};

} }