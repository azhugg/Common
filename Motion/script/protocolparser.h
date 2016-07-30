//
// ���ӵ� �޸𸮸� �����ϴ� ��ũ��Ʈ�� �м��Ѵ�.
// ����ü ������ ��ũ��Ʈ�� ó���� �� �ְ� �ߴ�.
//
// exmaple
// 4, float
// 4, float
// 4, float
// 4, float
// 4, float
//
//
#pragma once

#include "scriptutil.h"


struct sUDPField
{
	int bytes; // �����ϴ� �޸� ����Ʈ ũ��
	script::FIELD_TYPE::TYPE type; // ������ Ÿ��
};


class cProtocolParser
{
public:
	cProtocolParser();
	virtual ~cProtocolParser();

	bool Read(const string &fileName, const string &delimeter);
	bool ParseStr(const string &source, const string &delimeter= "\r\n");

	vector<sUDPField> m_fields;	
	int m_fieldsByteSize;
};
