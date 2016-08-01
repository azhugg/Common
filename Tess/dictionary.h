//
// 2016-07-31, jjuiddong
// ���� ���
//
#pragma once


namespace tess
{

	class cDictionary
	{
	public:
		cDictionary();
		virtual ~cDictionary();

		bool Init(const string &fileName);
		string FastSearch(const string &word, OUT vector<string> &out);
		string ErrorCorrection(const string &word);
		void Clear();


	protected:
		void GenerateCharTable();


	public:
		struct sSentence
		{
			string src; // ���� �ܾ�
			string lower; // �ҹ��� �ܾ�
		};

		vector<sSentence> m_sentences;
		map<string, int> m_sentenceLookUp; // sentence (lower case), sentence id
		map<string, int> m_wordLookup; // all separate word (lower case), int=m_words index
		vector<string> m_words; // lower case words
		char m_ambiguousTable[256]; // alphabet + number + special char
		int m_ambigId;

		struct sCharTable
		{
			vector<uint> words;
		};

		enum {
			MAX_CHAR=256,
			MAX_LEN = 30,
			MAX_WORD = 1000, // �ִ� �ܾ� ����
			MIN_WORD_LEN=3,
		};
		sCharTable m_charTable[MAX_CHAR][MAX_LEN]; // character table
		bool m_useChar[MAX_CHAR]; // ���ǰ� �ִ� ���ڶ�� true ���ȴ�. GenerateCharTable() ���� �ʱ�ȭ

		set<int> m_wordSet[MAX_WORD]; // �ܾ ���Ե� ������ ���� id �� �����Ѵ�.
	};

}
