//
// 2026-04-13, jjuiddong
// �ø��� ������� ��� ����� �������� ����� �Ѵ�.
// 
#pragma once

#include "output.h"

namespace motion
{
	
	class cSerialOutput : public cOutput
	{
	public:
		cSerialOutput();
		virtual ~cSerialOutput();

		bool Init(const int portNum, const int baudRate, const string &outputLog, const vector<string*> &formatScripts);
		virtual bool Start() override;
		virtual bool Stop() override;
		virtual bool Update(const float deltaSeconds) override;
		virtual void SendImmediate() override;
		virtual void SetFormat(const int index) override;


	public:
		int m_portNum;
		int m_baudRate;
		string m_outputLogFileName;
		bool m_isAutoClose; // Stop �� �ڵ����� �ø��� ��Ű� ����� �ȴ�.
		cSerialAsync m_serial;
		vector<cFormatParser*> m_formats;
	};

}
