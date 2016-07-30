//
// 2016-05-31, jjuiddong
// ������ ���е�, �ؽ�Ʈ ������ �м���, �׷��� ������ �����.
//
//	item1
//		item2
//			item3
//			item4
//		item5
//
#pragma once



namespace cvproc {
	namespace imagematch {

		class cGraphScript
		{
		public:
			cGraphScript();
			virtual ~cGraphScript();
			
			struct sNode
			{
				string id;
				string tag; // ���� label_id ó��
				int key; // keyboard
				bool check; // use traverse node
				bool noUpperTraverse; // use find route
				bool noProc; // use traverse node, no match scene
				bool isAuto; // use traverse node, no keyboard enter menu
				bool isSideMenu; // use traverse node, side menu operation
				float delay; // use move scene delay time seconds
				vector<sNode*> out;
				vector<sNode*> in;
			};

			bool Read(const string &fileName);
			sNode* Find(const string &id);
			sNode* Traverse(const string &id);
			bool FindRoute(const string &from, const string &to, OUT vector<sNode*> &out);
			bool FindRoute(sNode*current, const string &to, OUT vector<sNode*> &out);
			void CheckClearAllNode();
			void Clear();


		protected:
			sNode* build(sParseTree *parent, sParseTree *current, sNode *parentNode);
			sNode* FindParent(sNode *current, const string &id);
			sNode* FindParentRec(sNode *current, const string &id);
			sNode* TraverseRec(sNode *current, const string &id);
			bool FindRouteRec(sNode*current, const string &id, OUT vector<sNode*> &out);
			void buildAttributes(const sParseTree *node, const string &str, vector<string> &attributes);
			void setTreeAttribute(sParseTree *node, vector<string> &attribs);


		public:
			cParser2 m_parser;
			sNode *m_root;
			vector<sNode*> m_nodes;
		};

	}
}
