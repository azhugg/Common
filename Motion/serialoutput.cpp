
#include "stdafx.h"
#include "serialoutput.h"

using namespace motion;


cSerialOutput::cSerialOutput() 
	: cOutput(MOTION_MEDIA::SERIAL)
	, m_isAutoClose(true)
{
}

cSerialOutput::~cSerialOutput()
{
	for each (auto form in m_formats)
		delete form;
	m_formats.clear();
}


// bool cSerialOutput::Init(const int portNum, const int baudRate, const string &formatScript)
// {
// 	m_portNum = portNum;
// 	m_baudRate = baudRate;
// 
// 	if (!m_formatParser.Parse(formatScript))
// 		return false;
// 
// 	return true;
// }


bool cSerialOutput::Init(const int portNum, const int baudRate, const string &outputLog, const vector<string*> &formatScripts)
{
	m_portNum = portNum;
	m_baudRate = baudRate;
	m_outputLogFileName = outputLog;
	
	for each (auto scr in formatScripts)
	{
		cFormatParser *form = new cFormatParser();
		if (!form->Parse(*scr))
			return false;
		m_formats.push_back(form);
	}
	return true;
}


// �ø��� ��� ����
bool cSerialOutput::Start()
{
	if (!m_serial.Open(m_portNum, m_baudRate))
	{
		if (m_serial.IsOpen())
		{ 
			dbg::Log("serial already opend \n");
		}
		else
		{
			dbg::Log("Serial Open Error!! \n");
			return false;
		}
	}

	m_state = MODULE_STATE::START;
	
	// serial port, baudrate ����
	script::g_symbols["@output_serial_port"].type = script::FIELD_TYPE::T_INT;
	script::g_symbols["@output_serial_port"].iVal = m_portNum;
	script::g_symbols["@output_serial_baudrate"].type = script::FIELD_TYPE::T_INT;
	script::g_symbols["@output_serial_baudrate"].iVal = m_baudRate;

	// debug output log clear
	if (!m_outputLogFileName.empty())
		dbg::RemoveLog2(m_outputLogFileName.c_str());

	return true;
}


// �ø��� ��� ����
bool cSerialOutput::Stop()
{
	// �����ġ ���Ḧ �� ���� �ø�������� �ؾ��ϹǷ�, ����� ���� �ʴ� ��쵵 �ִ�.
	if (m_isAutoClose)
		m_serial.Close();

	m_state = MODULE_STATE::STOP;
	
	return true;
}


// �ø��� ��� ó��
bool cSerialOutput::Update(const float deltaSeconds)
{
	RETV(MODULE_STATE::STOP == m_state, false);
	RETV(!m_serial.IsOpen(), false);
	RETV(m_formats.empty(), false);
	RETV((int)m_formats.size() <= m_formatIdx, false);

	m_incTime += deltaSeconds;
	//if (m_incTime < 0.033f)
	//if (m_incTime < 0.039f)
	if (m_incTime < 0.049f)
		return true;

	BYTE buffer[128];
	const int len = m_formats[m_formatIdx]->Execute(buffer, sizeof(buffer));
	if (len > 0)
	{
		m_serial.SendData(buffer, len);

		if (!m_outputLogFileName.empty())
		{
			buffer[len] = NULL;
			dbg::Log2(m_outputLogFileName.c_str(), "%s \n", (char*)buffer);
		}
	}

	m_incTime = 0;
	return true;
}


// ��� ������ �۽��Ѵ�.
void cSerialOutput::SendImmediate()
{
	RET(m_formats.empty());
	RET((int)m_formats.size() <= m_formatIdx);

	BYTE buffer[128];
	const int len = m_formats[m_formatIdx]->Execute(buffer, sizeof(buffer));
	if (len > 0)
		m_serial.SendData(buffer, len);
}


void cSerialOutput::SetFormat(const int index)
{ 
	cOutput::SetFormat(index); 
	dbg::Log("Motion SerialOutput SetFormat %d \n", index);
}

