
#include "stdafx.h"
#include "vitconmotionsim2.h"



cVitconMotionSim2::cVitconMotionSim2() :
 m_state(OFF)
, m_serial(NULL)
, m_subState(SUBSTATE_OFF)
, m_playTime(0)
, m_delayTime(0)
{

}

cVitconMotionSim2::~cVitconMotionSim2()
{

}


bool cVitconMotionSim2::Init(cSerialAsync *serial)
{
	m_serial = serial;

	return true;
}


void cVitconMotionSim2::Update(const float deltaSeconds)
{

	switch (m_subState)
	{
	case SUBSTATE_OFF: State_Off(deltaSeconds); break;
	case SUBSTATE_INIT: State_Init(deltaSeconds); break;
	case SUBSTATE_START: State_Start(deltaSeconds); break;
	case SUBSTATE_ORIGIN: State_Origin(deltaSeconds); break;
	case SUBSTATE_ORIGIN_STOP: State_OriginStop(deltaSeconds); break;
	case SUBSTATE_STOP_READY: State_StopReady(deltaSeconds); break;
	case SUBSTATE_READY: State_Ready(deltaSeconds); break;
	case SUBSTATE_LOAD: State_Load(deltaSeconds); break;
	case SUBSTATE_PLAYING: State_Playing(deltaSeconds); break;
	case SUBSTATE_END: State_End(deltaSeconds); break;
	case SUBSTATE_END_STOP: State_EndStop(deltaSeconds); break;
	case SUBSTATE_RESTART: State_Restart(deltaSeconds); break;
	case SUBSTATE_STOP: State_Stop(deltaSeconds); break;
	case SUBSTATE_DELAY: State_Delay(deltaSeconds); break;

	case SUBSTATE_START_MOTION: m_subState = SUBSTATE_INIT; break;
	case SUBSTATE_STOP_MOTION: m_subState = SUBSTATE_END; break;
	default:
		break;
	}

}


// ��ǽùķ����͸� �Ѱ�, ServoOn -> Origin -> Stop ���·� �����.
// INIT -> ORIGIN -> READY
// SUBSTATE_START_MOTION -> SUBSTATE_INIT -> SUBSTATE_ORIGIN -> SUBSTATE_READY
void cVitconMotionSim2::On()
{
	m_state = INIT;
	m_subState = SUBSTATE_START_MOTION;
}


// �÷��̸� �����ϰ�, Origin���� ���ư���, Stop ���¿��� ����Ѵ�.
// STOP -> READY
void cVitconMotionSim2::Ready()
{
	m_subState = SUBSTATE_ORIGIN_STOP;
	m_playTime = 0;
}


// �÷��̸� �����ϰ�, Origin����, Stop ���¿��� ����Ѵ�.
// STOP -> READY
void cVitconMotionSim2::ReadyNoOrigin()
{
	m_playTime = 0;
	Delay(0.1f, SUBSTATE_STOP_READY);
}


// ��ǽùķ����͸� START ���·� �����.
bool cVitconMotionSim2::Play()
{
	switch (m_state)
	{
	case cVitconMotionSim2::OFF:
	case cVitconMotionSim2::INIT:
	case cVitconMotionSim2::END:
		return false; // �ƹ��� ����.

	case cVitconMotionSim2::READY_PLAY:
		return true;

	case cVitconMotionSim2::PLAY:
		if (SUBSTATE_DELAY == m_subState)
			m_subState = SUBSTATE_START; // ������ Stop ���� ���¶��, ���߰�, ������Ѵ�.
		return true;

	case cVitconMotionSim2::READY:
		m_subState = SUBSTATE_START; // �����ġ�� ���۽�Ų��. (Start ���·� �����.)
		return true;

	default: 
		return false;
	}
}


// �����ġ�� Stop ���·� �����.
void cVitconMotionSim2::Stop()
{
	switch (m_state)
	{
	case cVitconMotionSim2::OFF:
	case cVitconMotionSim2::INIT:
	case cVitconMotionSim2::END:
	case cVitconMotionSim2::READY:
		break; // �ƹ��� ����.

	case cVitconMotionSim2::READY_PLAY:
	case cVitconMotionSim2::PLAY:
		Ready(); // �����ġ�� Stop ��Ų��.
		break;
	}
}


// ��ǽùķ����͸� �����ϰ� ����. Stop (3 seconds delay) -> ServoOff
// END -> END_STOP
void cVitconMotionSim2::Off()
{
	m_state = END;
	m_subState = SUBSTATE_STOP_MOTION;
}


void cVitconMotionSim2::State_Off(const float deltaSeconds)
{
	m_state = OFF;
}


// Init State
// 1. ���� ȯ�漳�� ������ �д´�.
// 2. ��� �並 �ʱ�ȭ �ϰ�, Start ���·� �����ϰ� �Ѵ�.
void cVitconMotionSim2::State_Init(const float deltaSeconds)
{
	SendMotionSimMessage2(VITCON_SER::SERVOON);
	Delay(1, SUBSTATE_STOP_READY); // 1�� ��, Stop ���¿��� ����ϰ��Ѵ�.
}


void cVitconMotionSim2::State_Start(const float deltaSeconds)
{
	m_state = READY_PLAY;
	SendMotionSimMessage2(VITCON_SER::START);
	Delay(0.1f, SUBSTATE_PLAYING);
}


// Origin State
void cVitconMotionSim2::State_Origin(const float deltaSeconds)
{
	Delay(0, SUBSTATE_READY);
}


void cVitconMotionSim2::State_OriginStop(const float deltaSeconds)
{
	Delay(1, SUBSTATE_STOP_READY);
}


void cVitconMotionSim2::State_StopReady(const float deltaSeconds)
{
	SendMotionSimMessage2(VITCON_SER::STOP);
	Delay(3, SUBSTATE_READY);
}


void cVitconMotionSim2::State_Ready(const float deltaSeconds)
{
	// �ƹ��� ���� ���
	m_state = READY;
}


// Load State
void cVitconMotionSim2::State_Load(const float deltaSeconds)
{
	Delay(0.1f, SUBSTATE_START);
}


// Playing State
void cVitconMotionSim2::State_Playing(const float deltaSeconds)
{
	m_state = PLAY;
	m_playTime += deltaSeconds;
}


// End State
void cVitconMotionSim2::State_End(const float deltaSeconds)
{
	Delay(1, SUBSTATE_END_STOP);
}


// EndStop State
void cVitconMotionSim2::State_EndStop(const float deltaSeconds)
{
	SendMotionSimMessage2(VITCON_SER::STOP);
	Delay(3, SUBSTATE_STOP);
}


// Restart State
void cVitconMotionSim2::State_Restart(const float deltaSeconds)
{
}


// Stop State
void cVitconMotionSim2::State_Stop(const float deltaSeconds)
{
	SendMotionSimMessage2(VITCON_SER::SERVOOFF);
	Delay(1, SUBSTATE_OFF);
}


// Delay State
void cVitconMotionSim2::State_Delay(const float deltaSeconds)
{
	m_delayTime -= deltaSeconds;

	if (m_delayTime < 0)
	{
		m_subState = m_delayNextState;
	}
}


// Delay State
void cVitconMotionSim2::Delay(const float delaySeconds, const SUB_STATE next)
{
	if (next == SUBSTATE_DELAY)
		return;

	m_subState = SUBSTATE_DELAY;
	m_delayTime = delaySeconds;
	m_delayNextState = next;
}


string cVitconMotionSim2::GetStateStr() const
{
	string str;
	switch (m_state)
	{
	case cVitconMotionSim2::OFF: str = "Off"; break;
	case cVitconMotionSim2::INIT: str = "Init"; break;
	case cVitconMotionSim2::READY: str = "Ready"; break;
	case cVitconMotionSim2::READY_PLAY: str = "Ready Play"; break;
	case cVitconMotionSim2::PLAY: str = "Play"; break;
	case cVitconMotionSim2::END: str = "End"; break;
	default: str = "None"; break;
	}
	return str;
}


string cVitconMotionSim2::GetSubStateStr() const
{
	string str;
	switch (m_subState)
	{
	case cVitconMotionSim2::SUBSTATE_OFF: str = "Off"; break;
	case cVitconMotionSim2::SUBSTATE_START_MOTION: str = "Start_Motion"; break;
	case cVitconMotionSim2::SUBSTATE_INIT: str = "Init"; break;
	case cVitconMotionSim2::SUBSTATE_START: str = "Start"; break;
	case cVitconMotionSim2::SUBSTATE_ORIGIN: str = "Origin"; break;
	case cVitconMotionSim2::SUBSTATE_ORIGIN_STOP: str = "Origin_Stop"; break;
	case cVitconMotionSim2::SUBSTATE_STOP_READY: str = "Stop_Ready"; break;
	case cVitconMotionSim2::SUBSTATE_READY: str = "Ready"; break;
	case cVitconMotionSim2::SUBSTATE_LOAD: str = "Load"; break;
	case cVitconMotionSim2::SUBSTATE_PLAYING: str = "Playing"; break;
	case cVitconMotionSim2::SUBSTATE_END: str = "End"; break;
	case cVitconMotionSim2::SUBSTATE_END_STOP: str = "End_Stop"; break;
	case cVitconMotionSim2::SUBSTATE_RESTART: str = "Restart"; break;
	case cVitconMotionSim2::SUBSTATE_STOP_MOTION: str = "Stop_Motion"; break;
	case cVitconMotionSim2::SUBSTATE_STOP: str = "Stop"; break;
	case cVitconMotionSim2::SUBSTATE_DELAY: str = common::format("Delay %1.1f", m_delayTime); break;
	default: str = "None"; break;
	}
	return str;
}


// Dirt3 motion simulator �� ��������
void cVitconMotionSim2::SendMotionSimMessage2(const int state)
{
	RET(!m_serial);

	const int out_pitch = 10000;
	const int out_roll = 10000;
	const int out_yaw = 10000;
	const int out_sway = 10000;
	const int out_surge = 10000;
	const int out_heave = 10000;
	const int spareSpeed = 0;
	const int out_switch = state;

	// 	const string out = common::format(
	// 		"A%3d%3d%3d%3d%3d%3d%dZ", out_roll, out_pitch, out_yaw,
	// 		out_sway, out_surge, out_heave, out_switch);
	string out = common::format(
		"A%5d%5d%5d%5d%5d%5d%3d%3d%3d%3d%3d%3d%1dZ", out_roll, out_pitch, out_yaw,
		out_sway, out_surge, out_heave,
		spareSpeed, spareSpeed, spareSpeed, spareSpeed, spareSpeed, spareSpeed,
		out_switch);
	common::replaceAll(out, " ", "0"); // SpaceCharToZeroChar

	for (int i = 0; i < 5; ++i)
	{
		Sleep(50);
		m_serial->SendData((BYTE*)out.c_str(), out.size());
	}
	Sleep(50);
}