//
// 2016-04-17, jjuiddong
// motion input �� mixer ���̿��� �����Ѵ�.
//
#pragma once


namespace motion
{

	class cModule
	{
	public:
		cModule() : m_state(MODULE_STATE::STOP), m_incTime(0) {}
		virtual ~cModule() {}

		virtual bool Start() = 0;
		virtual bool Stop() = 0;
		virtual bool Update(const float deltaSeconds) = 0;


	public:
		MODULE_STATE::TYPE m_state;
		float m_incTime;
	};

}
