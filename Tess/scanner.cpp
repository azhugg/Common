
#include "stdafx.h"
#include "scanner.h"
#include "dictionary.h"

using namespace tess;


tess::cScanner::cScanner()
{
}

cScanner::cScanner(const cDictionary &dict, const string &str)
{
	Init(dict, str);
}

tess::cScanner::~cScanner()
{
}


// �ҹ��� ��ȯ, ���鹮�ڴ� �ִ� 1�� ����, 
// ������� �ʴ� Ư�� ���� ����
bool cScanner::Init(const cDictionary &dict, const string &str)
{
	string src = str;
	trim(src);
	lowerCase(src);

	m_str.clear();

	// ���̴� ����, ��ȣ�� ��ȣ�� ������ ���� ����
	for (uint i = 0; i < src.length(); ++i)
	{
		const char c = src[i];
		if ((dict.m_useChar[c]) || (dict.m_ambiguousTable[c] != 0))
			m_str += c;
	}
	
	// ���� �ִ� 1��
	bool isBlank = false;
	for (uint i = 0; i < m_str.length(); )
	{
		if (m_str[i] == ' ')
		{
			if (isBlank)
			{ // ���鹮�ڸ� ����ڷ� �ű� ��, ����
				common::rotatepopvector(m_str, i);
			}
			else
			{
				++i;
				isBlank = true;
			}
		}
		else
		{
			++i;
			isBlank = false;
		}
	}

	return true;
}


bool cScanner::IsEmpty()
{
	return m_str.empty();
}
