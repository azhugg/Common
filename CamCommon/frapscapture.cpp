
#include "stdafx.h"
#include "frapscapture.h"

using namespace cv;
using namespace cvproc;

void FrapsThreadProc(cFrapsCapture*);


cFrapsCapture::cFrapsCapture()
	: m_state(WAIT)
	, m_keyDownDelay(100)
{
	m_exts.push_back("bmp");
	m_exts.push_back("jpg");
}

cFrapsCapture::~cFrapsCapture()
{
	Clear();
}


bool cFrapsCapture::Init(const string &frapsScreenShotPath, const int key)
{
	Clear();

	m_frapsScreenShotPath = frapsScreenShotPath;
	m_key = key;

	m_isRunThread = true;
	m_thread = std::thread(FrapsThreadProc, this);

	return true;
}


void cFrapsCapture::ScreenShot()
{
	m_state = SCREENSHOT;
}


// ��ũ������ ������Ʈ �Ǿ��ٸ�, true�� ����
bool cFrapsCapture::IsUpdateScreenShot()
{
	return (m_state == END);
}


void cFrapsCapture::Clear()
{
	if (m_thread.joinable())
	{
		m_isRunThread = false;
		m_thread.join();
	}
}


void cFrapsCapture::KeyDown()
{
	INPUT input;
	//WORD vkey = VK_SNAPSHOT; // see link below
	WORD vkey = m_key;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = vkey;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));

	Sleep(m_keyDownDelay);

	input.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));

	Sleep(m_keyDownDelay);
}


// Fraps ��ũ���� ��ο� ������ ������Ʈ �Ǿ����� Ȯ���Ѵ�.
// Ȯ�ε� ������ �ε��ϰ� �����. ������ ��ũ���� ���ϵ鵵 �����.
// *.bmp ������ ã�´�.
//
// return value : ������ ã�Ҵٸ� true�� ����.
//
bool cFrapsCapture::SearchFrapsDir()
{
	list<string> out;
	CollectFiles(m_exts, m_frapsScreenShotPath, out);

	if (out.empty())
		return false;

	m_img = imread(out.back());

	for each (auto file in out)
		DeleteFileA(file.c_str());

	return true;
}


// Fraps ���丮�� �˻��ؼ�, ��ũ���� �̹����� ������ �����´�.
void FrapsThreadProc(cFrapsCapture *capture)
{
	while (capture->m_isRunThread)
	{

		switch (capture->m_state)
		{
		case cFrapsCapture::WAIT:  // �����
			Sleep(30);
			break;

		case cFrapsCapture::SCREENSHOT:	// ��ũ���� ��û
			capture->m_state = cFrapsCapture::KEYDOWN;
			break;

		case cFrapsCapture::KEYDOWN: // ��ũ���� Ű �ٿ�
			capture->KeyDown();
			capture->m_state = cFrapsCapture::SEARCH_DIR;
			break;

		case cFrapsCapture::SEARCH_DIR: // Fraps ���丮 �˻�
		{
			int cnt = 0; // 2�� ���� �˻��Ѵ�.
			while (!capture->SearchFrapsDir() && (cnt < 20))
			{
				++cnt;
				Sleep(30);
			}
			capture->m_state = cFrapsCapture::END;
		}
		break;

		case cFrapsCapture::END: // ��ũ���� �̹��� �������� ����.
			Sleep(30);
			break;

		default:
			assert(0);
			break;
		}

	}
}
