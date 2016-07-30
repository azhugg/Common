#pragma once

/*
#ifdef DLL_EXPORT
	#define DLL_FUNC __declspec(dllexport)
#else
	#define DLL_FUNC __declspec(dllimport)
#endif


extern "C"
{
	// ��� ��ũ��Ʈ ������ �д� �Լ�
	// �����ϸ� 1�� �����ϰ�, �׷��� ������ 0�� �����Ѵ�.
	DLL_FUNC int MotionInit(int hWnd, char *fileName);


	// �� ���������� ȣ���ؾ� �ϴ� �Լ�
	// deltaSeconds : 1�ʰ� ������ 1
	DLL_FUNC void MotionUpdate(const float deltaSeconds);


	// �ʱ�ȭ
	DLL_FUNC void Clear();


	// �����ġ�� ������ �õ��Ѵ�.
	// �����ġ�� On ��Ű��, Stop ���¿��� ����Ѵ�.
	// �����ϸ� 1�� �����ϰ�, �׷��� ������ 0�� �����Ѵ�.
	DLL_FUNC int MotionStart();


	// �����ġ�� Off ��Ų��.
	// �����ġ�� ������ ��������.
	// �����ϸ� 1�� �����ϰ�, �׷��� ������ 0�� �����Ѵ�.
	DLL_FUNC int MotionStop();


	// ��� ������ �����Ѵ�.
	// :output - :format �� �����Ѵ�.
	// outputIndex : :output �� 2�� �̻��� ���, ��� ��ũ��Ʈ���� ������ ������� 0,1~
	// �� ��������.
	// formatIndex : :format �� 2�� �̻��� ���, ��� ��ũ��Ʈ���� ������ ������� 0,1~
	// �� ��������.
	DLL_FUNC void MotionSetOutputFormat(int outputIndex, int formatIndex);


	// ��� ��ũ��Ʈ Varible�� float������ ��ȯ�ؼ� �����´�.
	DLL_FUNC float MotionGetSymbolFloat(char *symbolName);


	// ��� ��ũ��Ʈ Varible�� int������ ��ȯ�ؼ� �����´�.
	DLL_FUNC int MotionGetSymbolInt(char *symbolName);


	// ��� ��ũ��Ʈ Varible�� float������ �����Ѵ�.
	DLL_FUNC void MotionSetSymbolFloat(char *symbolName, float val);


	// ��� ��ũ��Ʈ Varible�� int������ �����Ѵ�.
	DLL_FUNC void MotionSetSymbolInt(char *symbolName, int val);



	//----------------------------------------------------------------------------
	// �̹��� �ν� �ʱ�ȭ
	DLL_FUNC int MatchReadScript(char *fileName, char *executeLabel);

	// �̹��� ĸ��, �ν��ϱ�
	DLL_FUNC void MatchCaptureAndDetect(char *result);

	// �̹��� ĸ�� ����
	DLL_FUNC void MatchCaptureDelay(int delayMilliSeconds);

}
*/
