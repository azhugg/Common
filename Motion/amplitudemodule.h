//
// 2016-07-12, jjuiddong
//
// ��� ������ ũ�⸦ �����ϴ� ���.
// ������ ������ ũ��, ������ ���ߴ� ����� �Ѵ�.
//
// �Է�:
//		@roll_fin, @pitchl_fin,  @yaw_fin
//		3���� ũ�⸦ �Է����� �Ѵ�.
//
// ���: 
//		@amplitude : ��ȯ�� �������� ��Ÿ��
//		@amplitude0 : ���� �������� ��Ÿ��
//		@roll_fin, @pitchl_fin,  @yaw_fin : ���� ���� ��
//
//
#pragma once


namespace motion
{

	class cAmplitudeModule : public cModule
	{
	public:
		cAmplitudeModule();
		virtual ~cAmplitudeModule();

		bool Init(const string &mathScript, const string &modulatorScript);
		virtual bool Start() override;
		virtual bool Stop() override;
		virtual bool Update(const float deltaSeconds) override;


	public:
		Vector3 m_prevAxis; // previews roll, pitch, yaw
		cMathParser m_mathParser;
		mathscript::cMathInterpreter m_matInterpreter;
		cSimpleModulator m_modulator;
	};

}