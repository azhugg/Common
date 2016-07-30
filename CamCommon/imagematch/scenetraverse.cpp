
#include "stdafx.h"
#include "scenetraverse.h"
#include "matchprocessor.h"

using namespace cvproc;
using namespace cvproc::imagematch;

cSceneTraverse::cSceneTraverse()
	: m_state(WAIT)
	, m_currentNode(NULL)
	, m_nextNode(NULL)
	, m_delayTime(0)
	, m_isInitDelay(false)
	, m_initDelayTime(0)
	, m_nextMenuIdx(0)
	, m_currentMenuIdx(0)
	, m_screenCaptureKey(VK_SNAPSHOT)
	, m_genId(100000)
	, m_matchResultBuffIdx(0)
	, m_isLog(false)
{
}

cSceneTraverse::~cSceneTraverse()
{
}


bool cSceneTraverse::Init(const string &imageMatchScriptFileName, 
	const string &flowScriptFileName
	, const int screenCaptureKey) // screenCaptureKey=VK_SNAPSHOT
{
	Cancel();

	cMatchProcessor::Get()->m_isLog = false;
	if (!m_matchScript.Read(imageMatchScriptFileName))
	{
		dbg::ErrLog("Error!! cSceneTraverse::Init(), read error image match script \n");
		return false;
	}

	if (!m_flowScript.Read(flowScriptFileName))
	{
		dbg::ErrLog("Error!! cSceneTraverse::Init(), read error menu script \n");
		return false;
	}

	m_screenCaptureKey = screenCaptureKey;

	return true;
}


// �� ���������� ȣ�� �Ǿ�� �Ѵ�.
// 
cSceneTraverse::STATE cSceneTraverse::Update(const float deltaSeconds, const cv::Mat &img, OUT int &key)
{
	STATE nextState = m_state;

	switch (m_state)
	{
	case WAIT:
		break;

	case DELAY:
		nextState = OnDelay(deltaSeconds, img, key);
		break;

	case REACH:
		break;

	case CAPTURE:
		nextState = OnCapture(img, key);
		break;

	case PROC:
		nextState = OnProc(img, key);
		break;

	case MENU_MOVE:
		nextState = OnMenu(img, key);
		break;

	case MENU_DETECT:
		nextState = OnMenuDetect(img, key);
		break;

	case ERR:
		break;

	default: 
		assert(0); 
		break;
	}

	m_state = nextState;

	return nextState;
}


cSceneTraverse::STATE cSceneTraverse::OnDelay(const float deltaSeconds, const cv::Mat &img, OUT int &key)
{
	// ���������� ��ŵ�ϰ�, ���� ������ ���� ����
	// ��Ī ��� ������, deltaSeconds ���� ū ��츦 �Ѿ�� ���� �ڵ�.
	if (m_isInitDelay)
	{
		m_isInitDelay = false;
		return DELAY;
	}

	m_delayTime -= deltaSeconds;
	if (m_delayTime < 0)
	{
		return m_nextState;
	}

	return DELAY;
}


cSceneTraverse::STATE cSceneTraverse::OnCapture(const cv::Mat &img, OUT int &key)
{
	key = m_screenCaptureKey;
	return PROC;
}


// �̹��� ���� ó��
// img �� �ν���, ���� ���� �Ǵ��ϰ�, ��ǥ ������ �̵��Ѵ�.
cSceneTraverse::STATE cSceneTraverse::OnProc(const cv::Mat &img, OUT int &key)
{
	if (!img.data)
		return PROC;

	if (m_nextNode && m_nextNode->noProc)
	{
		// �� ��Ī�� ���� �ʰ�, Ű���� ���͸� ���� ��, 
		// ���� ������ �Ѿ��.
		m_currentNode = m_nextNode;

		// ��ǥ ��ġ���� ��� Ž��
		m_path.clear();
		if (m_flowScript.FindRoute(m_nextNode, m_moveTo, m_path))
		{
			if (m_path.empty() || (m_path.size() == 1))
			{
				// �̹� ������ ����
				return REACH;
			}

			// ���� ��带 �����Ѵ�.
			if (m_path.size() > 1)
			{
				m_nextNode = m_path[1];
			}
		}
		else
		{
			dbg::ErrLog("Error!! cSceneTraverse::OnProc(), not found route [%s] \n", m_moveTo.c_str());
			return ERR;
		}

		key = (m_nextNode->key)? m_nextNode->key : VK_RETURN;
		Delay(m_currentNode->delay, CAPTURE);
		return m_state;
	}


	const int t1 = timeGetTime();

	sTreeMatchData data;
	data.node = NULL;
	data.loopCnt = 0;

	if (m_nextNode)
	{
		TreeMatch(m_nextNode, img.clone(), data, 1);
	}
	else
	{
		cMatchResult &matchResult = GetMatchResult();
		matchResult.Init(&m_matchScript, &img, "", 0,
			(sParseTree*)m_matchScript.FindTreeLabel("@detect_scene"),
			true, true);
		cMatchProcessor::Get()->Match(matchResult);
		
		data.result = matchResult.m_resultStr;
		data.node = m_flowScript.Find(matchResult.m_resultStr);

 		if (m_isLog)
 			dbg::Log("cSceneTraverse::OnProc result=%s, loop=%d, time=%d\n", data.result.c_str(), data.loopCnt, timeGetTime()-t1);
	} 

	// �νĿ� �����ϸ�, �ٽ� ������ �ν��Ѵ�.
	if (!data.node)
	{
		++m_tryMachingCount;
		if (m_tryMachingCount > 2)
		{
			m_tryMachingCount = 0;
			m_nextNode = NULL; // ��ü ���� ��Ī�Ѵ�.
		}

		key = m_screenCaptureKey;
		return PROC;
	}

	m_currentNode = data.node;
	m_nextNode = NULL;
	m_tryMachingCount = 0;

	// ��ǥ ��ġ���� ��� Ž��
	m_path.clear();
	if (m_flowScript.FindRoute(data.node, m_moveTo, m_path))
	{
		if (m_path.empty() || (m_path.size()==1))
		{
			// �̹� ������ ����
			return REACH;
		}

		// ���� ��带 �����Ѵ�.
		if (m_path.size() > 1)
		{
			m_nextNode = m_path[1];
		}
	}
	else
	{
		dbg::ErrLog("Error!! cSceneTraverse::OnProc(), not found route [%s] \n", m_moveTo.c_str());
		return ERR;
	}

	// ���� ������ �̵��� ���� Ű ����
	const bool IsUpMenu = CheckMenuUpandDown(m_path[0], m_path[1]);

	// goto parent menu
	if (IsUpMenu)
	{
		key = VK_ESCAPE;
		Delay(3.f, CAPTURE);
		return m_state;
	}

	// ���� �ִ� �޴��� �ϳ� �̻��� ���, �޴��� �����ϴ� ���·� �Ѿ��.
 	m_currentMenuIdx = 0;
 	m_nextMenuIdx = 0;
	key = 0;

	const int nextMenuCount = GetNextMenuCount(m_path[0], m_path[1]);
	const int nextMenuIdx = CheckNextMenuIndex(m_path[0], m_path[1]);
	if (nextMenuIdx < 0) // next parent menu
	{
		key = VK_ESCAPE;
		Delay(3.f, CAPTURE);
	}
	else if (nextMenuCount == 1) // ���� �޴��� �ϳ��ۿ� ���� ���..
	{
		key = (m_nextNode->key) ? m_nextNode->key : VK_RETURN;
		Delay(m_currentNode->delay, CAPTURE);
	}
	else if (nextMenuIdx >= 0)
	{
		return OnMenuDetect(img, key);
	}

	return m_state;
}


cSceneTraverse::STATE cSceneTraverse::OnMenu(const cv::Mat &img, OUT int &key)
{
	switch (m_state)
	{
	case MENU_MOVE:
		if (m_currentMenuIdx == m_nextMenuIdx)
		{
			key = VK_RETURN;
			Delay(m_nextNode->delay, CAPTURE);
		}
		else
		{
			if (m_currentMenuIdx < m_nextMenuIdx)
			{
				++m_currentMenuIdx;
				key = (m_currentNode->isSideMenu)? VK_RIGHT : VK_DOWN;
			}
			else
			{
				--m_currentMenuIdx;
				key = (m_currentNode->isSideMenu)? VK_LEFT : VK_UP;
			}

			Delay(0.3f, MENU_MOVE);
		}
		break;

	case MENU_DETECT:
	case WAIT:
	case DELAY:
	case REACH:
	case CAPTURE:
	case PROC:
	case ERR:
		break;

	default: 
		assert(0); 
		break;
	}

	return m_state;
}


// ���� �޴��� ��ġ�� Ȯ���ϰ�, ��ǥ �޴��� �̵��Ѵ�.
// Left, Right Button or Up, Down Button
cSceneTraverse::STATE cSceneTraverse::OnMenuDetect(const cv::Mat &img, OUT int &key)
{
	cGraphScript::sNode *node = m_currentNode;
	if (!node)
		node = m_flowScript.m_root;

	cMatchResult &matchResult = GetMatchResult();
	const string label = node->tag.empty() ? node->id : node->tag;
	matchResult.Init(&m_matchScript, &img, "", 0,
		(sParseTree*)m_matchScript.FindTreeLabel(string("@") + label + "_menu"),
		true, true);
	matchResult.m_traverseType = 1;

	cMatchProcessor::Get()->Match(matchResult);

	if (m_isLog)
		dbg::Log("cSceneTraverse::OnMenuDetect label=%s, result=%s\n", label.c_str(), matchResult.m_resultStr.c_str());

	if (matchResult.m_result <= 0)
	{
		// �νĿ� �����ϸ� �ٽ� �õ��Ѵ�
		return CAPTURE;
	}

	const string selectMenuId = matchResult.m_resultStr;
	if (selectMenuId == m_nextNode->id)
	{
		// �������� �Ѿ �޴��� �����ϰ� �ִ� ����
		// ����Ű�� ���� ���� ������ �Ѿ��.
		key = VK_RETURN;
		Delay(m_nextNode->delay, CAPTURE);
	}
	else
	{
		m_nextMenuIdx = CheckNextMenuIndex(m_path[0], selectMenuId, m_path[1]);
		m_currentMenuIdx = 0;

		if (m_nextMenuIdx == 0)
		{
			// ����, ���� ��ġ�� �ٽ� �ľ��� ��, �����Ѵ�.
			return CAPTURE;
		}

		// m_nextMenuIdx ��ŭ �޴��� �̵��Ѵ�.
		return MENU_MOVE;
	}

	return m_state;
}


void cSceneTraverse::Delay(const float delaySeconds, const STATE nextState)
{
	m_isInitDelay = true;
	m_initDelayTime = delaySeconds;
	m_delayTime = delaySeconds;
	m_nextState = nextState;
	m_state = DELAY;
}


// �� �̵� ����� ����, ������ ĸ����, ���� ��ġ �ľ��� ���� �Ѵ�.
void cSceneTraverse::Move(const string &sceneName)
{
	Cancel();

	if (!sceneName.empty())
	{
		m_moveTo = sceneName;
		m_state = CAPTURE;
	}
}


// �޴� �̵� ����
void cSceneTraverse::Cancel()
{
	m_state = WAIT;
	m_currentNode = NULL;
	m_nextNode = NULL;
	m_nextMenuIdx = 0;
	m_currentMenuIdx = 0;
	m_tryMachingCount = 0;
	m_path.clear();
	m_moveTo.clear();
}


string cSceneTraverse::GenerateInputID()
{
	stringstream ss;
	ss << "@input" << m_genId++;
	if (m_genId > 100000)
		m_genId = 0;
	return ss.str();
}


cMatchResult& cSceneTraverse::GetMatchResult()
{
	const int MAX_LEN = sizeof(m_matchResult) / sizeof(cMatchResult);

	++m_matchResultBuffIdx;
	if (m_matchResultBuffIdx >= MAX_LEN)
		m_matchResultBuffIdx = 0;
	return m_matchResult[m_matchResultBuffIdx];
}


bool cSceneTraverse::TreeMatch(cGraphScript::sNode *current, const cv::Mat &img, 
	OUT sTreeMatchData &out, const int loopCnt) // loopCnt=-1
{
	RETV(!current, false);

	// �ߺ� ��ȸ�� �������� ���δ�.
	m_flowScript.CheckClearAllNode();

	queue<cGraphScript::sNode*> q;
	q.push(current);

	const string inputName = GenerateInputID();

	vector<sMatchInfo> matchInfo;
	cMatchProcessor::Get()->SetInputImage(img, inputName);

	// breath first search
	int loop = 0;
	string result;
	while (!q.empty() && ((loopCnt==-1) || (loop < loopCnt)))
	{
		cGraphScript::sNode *node = q.front();
		q.pop();

		if (node->check) // �̹� �ѹ� ���������, �Ѿ��.
			continue;

		cMatchResult &matchResult = GetMatchResult();
		const string label = node->tag.empty() ? node->id : node->tag;
		matchResult.Init(&m_matchScript, &img, inputName, 0,
			(sParseTree*)m_matchScript.FindTreeLabel(string("@") + label),
			true, true);

		if (m_isLog)
			dbg::Log("cSceneTraverse::TreeMatch label = %s\n", label.c_str());

		cMatchProcessor::Get()->Match(matchResult);
		result = matchResult.m_resultStr;

		if (m_isLog)
			dbg::Log("    - result = %s \n", result.c_str());

		if ((result != "~ fail ~") && (result != "not found label"))
		{
			out.node = node;
			out.result = result;
			out.loopCnt = loop;
			return true; // success find~ return
		}

		node->check = true;

		// �Ʒ��� ���� �����ؼ�, ���� �˻��ϰ� �Ѵ�. (���� �ö󰥼��� �ӵ��� ����, main, option)
		for each (auto p in node->out)
		{
			if (!p->check)
				q.push(p);
		}

		if (loopCnt == -1) // ��ü�� �˻��� ����, ���� �ö󰡼� �˻��Ѵ�.
		{
			for each (auto p in node->in)
			{
				if (!p->check)
					q.push(p);
			}
		}

		if (result != "not found label") // ���� ���� ���� �˻��� ������ �������� �ʴ´�.
			++loop;
	}

	out.loopCnt = loop;
	return false;
}


// ���� �޴��� �ö󰡸�, true �� �����ϰ�,
// ���� �޴��� ��������, false�� �����Ѵ�.
bool cSceneTraverse::CheckMenuUpandDown(
	const cGraphScript::sNode *current, 
	const cGraphScript::sNode *next)
{
	for each (auto p in current->out)
	{
		if (p == next)
			return false;
	}

	for each (auto p in current->in)
	{
		if (p == next)
			return true;
	}

	return true;
}


// ���� ��忡�� �������� �Ѿ �� �����ؾ� ��, �б� �޴� ������ �����Ѵ�.
int cSceneTraverse::GetNextMenuCount(
	const cGraphScript::sNode *current, 
	const cGraphScript::sNode *next)
{
	// menu down
	bool isMenuDown = false;
	int cnt1 = 0;
	for each (auto p in current->out)
	{
		if (p == next)
			isMenuDown = true;
		if (!p->isAuto)
			++cnt1;
	}
	if (isMenuDown)
		return cnt1;

	// menu up
	bool isMenuUp = false;
	int cnt2 = 0;
	for each (auto p in current->in)
	{
		if (p == next)
			isMenuUp = true;
		if (!p->isAuto)
			++cnt2;
	}
	if (isMenuUp)
		return cnt2;

	return 0;
}


// node 
// �����̸�, �Ʒ����� ���� �ö󰡰ų�, �����ʿ��� �������� �̵��ϴ� �޴��� ���Ѵ�.
int cSceneTraverse::CheckNextMenuIndex(const cGraphScript::sNode *current, 
	const cGraphScript::sNode *next)
{
	RETV(!current || !next, 0);

	int cnt1 = 0;
	for each (auto p in current->out)
	{
		if (p == next)
		{
			return cnt1;
		}
		++cnt1;
	}

	int cnt2 = 0;
	for each (auto p in current->in)
	{
		if (p == next)
		{
			return cnt2;
		}
		--cnt2;
	}

	return 0;
}


// �����̸�, �Ʒ����� ���� �ö󰡰ų�, �����ʿ��� �������� �̵��ϴ� �޴��� ���Ѵ�.
int cSceneTraverse::CheckNextMenuIndex(
	const cGraphScript::sNode *current,
	const string &selectMenuId,
	const cGraphScript::sNode *next)
{
	RETV(!current || !next, 0);

	int targetMenuIndex = 0;
	for each (auto p in current->out)
	{
		if (p == next)
			break;
		++targetMenuIndex;
	}

	int currentMenuIndex = 0;
	for each (auto p in current->out)
	{
		if (p->id == selectMenuId)
			break;
		++currentMenuIndex;
	}

	return targetMenuIndex - currentMenuIndex;
}

