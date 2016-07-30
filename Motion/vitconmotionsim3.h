#pragma once
//
// Dirt3 ���̽� ��� �ùķ����� �� VITCON ���� Version.3 ��� Ŭ����
//
// StateMachine ��� ���� ����
//
#pragma once


class cVitconMotionSim3
{
public:
	cVitconMotionSim3();
	virtual ~cVitconMotionSim3();

	enum STATE {
		OFF,	// SUBSTATE_OFF�� ��
		INIT,	// On() �� ��
		READY,  // SUBSTATE_READY�� �� 
		READY_PLAY, // SUBSTATE_READY���� SUBSTATE_PLAYING���� �Ѿ�� ����
		PLAY,	// SUBSTATE_PLAYING�� ��
		READY_STOP,
		END,	// Off() �� ��
	};

	enum SUB_STATE {
		SUBSTATE_OFF,
		SUBSTATE_START_MOTION,
		SUBSTATE_INIT, // Servo On (5 seconds) 
		SUBSTATE_START, // Start (1 seconds)
		SUBSTATE_ORIGIN, // Origin Position (5 seconds)
		SUBSTATE_ORIGIN_STOP, // Origin Position (5 seconds) -> Stop
		SUBSTATE_STOP_READY, // Stop -> Ready
		SUBSTATE_READY, // Wait
		SUBSTATE_LOAD, // Load MotionWave
		SUBSTATE_PLAYING, // Play MotionWave
		SUBSTATE_END, // Origin Position(5 seconds)
		SUBSTATE_END_STOP, // Stop
		SUBSTATE_RESTART, // Origin Position(5 seconds) -> Start
		SUBSTATE_STOP_MOTION,
		SUBSTATE_STOP, // Servo Off
		SUBSTATE_DELAY,

		// VitCon ��� �ùķ����� ���� ����
		// INIT -> SUBSTATE_STOP_READY -> READY

		// ���ӽ���	
		// LOAD -> START -> PLAYING -> END

		// ��������, ���
		// ORIGIN -> STOP -> READY

		// �� ����
		// RESTART -> LOAD -> PLAYING -> END

		// ����
		// END -> END_STOP
	};


public:
	bool Init(cSerialAsync *serial);
	void Update(const float deltaSeconds);
	void On();
	void Ready();
	void ReadyNoOrigin();
	bool Play();
	void Stop();
	void Off();

	STATE GetState() const;
	SUB_STATE GetSubState() const;
	string GetStateStr() const;
	string GetSubStateStr() const;
	float GetPlayTime() const;


protected:
	void State_Off(const float deltaSeconds);
	void State_Init(const float deltaSeconds);
	void State_Start(const float deltaSeconds);
	void State_Origin(const float deltaSeconds);
	void State_OriginStop(const float deltaSeconds);
	void State_StopReady(const float deltaSeconds);
	void State_Ready(const float deltaSeconds);
	void State_Load(const float deltaSeconds);
	void State_Playing(const float deltaSeconds);
	void State_End(const float deltaSeconds);
	void State_EndStop(const float deltaSeconds);
	void State_Restart(const float deltaSeconds);
	void State_Stop(const float deltaSeconds);
	void State_Delay(const float deltaSeconds);
	void Delay(const float delaySeconds, const SUB_STATE next);
	void SendMotionSimMessage2(const int state);


protected:
	STATE m_state;
	SUB_STATE m_subState;
	cSerialAsync *m_serial;

	float m_playTime; // seconds
	float m_delayTime;
	SUB_STATE m_delayNextState;
};


inline cVitconMotionSim3::STATE cVitconMotionSim3::GetState() const { return m_state; }
inline cVitconMotionSim3::SUB_STATE cVitconMotionSim3::GetSubState() const { return m_subState; }
inline float cVitconMotionSim3::GetPlayTime() const { return m_playTime; }
