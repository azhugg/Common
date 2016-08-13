
#include "stdafx.h"
#include "graphscript.h"


using namespace cvproc;
using namespace cvproc::imagematch;

cGraphScript::cGraphScript()
{
}

cGraphScript::~cGraphScript()
{
	Clear();
}


bool cGraphScript::Read(const string &fileName)
{
	Clear();

	if (!m_parser.Read(fileName))
		return false;

	m_root = build(NULL, m_parser.m_treeRoot, NULL);

	return true;
}


void cGraphScript::buildAttributes(const sParseTree *node, const string &str, vector<string> &attributes)
{
	m_parser.m_lineStr = (char*)str.c_str();
	while (1)
	{
		const char *pid = m_parser.id();
		const char *pnum = (!*pid) ? m_parser.number() : NULL;

		if (*pid)
		{
			const string symb = m_parser.GetSymbol(pid);
			if (symb.empty())
			{
				attributes.push_back(pid);
			}
			else
			{
				char *old = m_parser.m_lineStr;
				buildAttributes(node, symb, attributes);
				m_parser.m_lineStr = old;
			}
		}
		else if (*pnum)
		{
			attributes.push_back(pnum);
		}
		else
		{
			dbg::ErrLog("line {%d} error!! tree attribute fail [%s]\n", node->lineNum, str.c_str());
			m_parser.m_isError = true;
			break;
		}

		// comma check
		m_parser.m_lineStr = m_parser.passBlank(m_parser.m_lineStr);
		if (!m_parser.m_lineStr)
			break;
		if (*m_parser.m_lineStr == ',')
			++m_parser.m_lineStr;
	}
}


// id,1 : �ڽ� ��忡�� ���� ���� �ö�� �� ����. no upper traverse
void cGraphScript::setTreeAttribute(sParseTree *node, vector<string> &attribs)
{
	if (attribs.empty())
		return;

	strcpy_s(node->name, attribs[0].c_str());

	// check delay
	node->delay = 0.5f; // default

	// ù��° �Ӽ����� node id �̹Ƿ� ���õȴ�.
	for (int i = 1; i < (int)attribs.size(); ++i)
	{
		if (string::npos != attribs[i].find("delay_"))
		{
			// delay_num, delay  �� ����
			const int valPos = attribs[i].find("_");
			const float delay = (float)atof(attribs[i].substr(valPos + 1).c_str());
			node->delay = delay;
		}
		else if (string::npos != attribs[i].find("tag_"))
		{
			// tag_id, tag �� ����
			const int valPos = attribs[i].find("_");
			const string tag = attribs[i].substr(valPos + 1);
			strcpy_s(node->tag, tag.c_str());
		}
		else if (string::npos != attribs[i].find("noproc"))
		{
			// noproc
			node->noProc = true;
		}
		else if (string::npos != attribs[i].find("noup"))
		{
			// noup
			node->noUpperTraverse = true;
		}
		else if (string::npos != attribs[i].find("auto"))
		{
			// auto
			node->isAuto = true;
		}
		else if (string::npos != attribs[i].find("key"))
		{
			// key_id �� ����
			const int valPos = attribs[i].find("_");
			const string keyboard = attribs[i].substr(valPos + 1);
			if (keyboard == "esc")
				node->key = VK_ESCAPE;
			else if (keyboard == "enter")
				node->key = VK_RETURN;
		}
		else if (string::npos != attribs[i].find("sidemenu"))
		{
			// sidemenu
			node->isSideMenu = true;
		}
		else if (string::npos != attribs[i].find("enterchild"))
		{
			// enterchild
			node->isEnterChild = true;
		}
		else if (string::npos != attribs[i].find("nomenu"))
		{
			// nomenu
			node->isNoMenu = true;
		}
		else if (string::npos != attribs[i].find("sidesel"))
		{
			// sidesel
			node->isSideSubmenu = true;
		}
		else if (string::npos != attribs[i].find("circular"))
		{
			// circular
			node->isCircularMenu = true;
		}
		else if (string::npos != attribs[i].find("sceneid_"))
		{
			// sceneid_~~
			// sceneid_inherit �θ� ���� ���� ��ӹ޴´�. ������ ID �� ����.
			// sceneid_child_inherit, ���� ����� Scene ID�� �ڽ� ��忡�� ��ӽ�Ų��.
			if (attribs[i] == "sceneid_inherit")
			{
				node->isSceneIdInherit = true;
			}
			else if (attribs[i] == "sceneid_child_inherit")
			{
				node->isSceneIdChildInherit = true;
			}
			else
			{
				const int valPos = attribs[i].find("_");
				const int id = atoi(attribs[i].substr(valPos + 1).c_str());
				node->sceneId = id;
			}
		}
	}

}


cGraphScript::sNode* cGraphScript::build(sParseTree *parent, sParseTree *current, sNode *parentNode)
{
	RETV(!current, NULL);

	vector<string> attribs;
	buildAttributes(current, current->line, attribs);
	setTreeAttribute(current, attribs);

	sNode *linkNode = FindParent(parentNode, current->name);
	sNode *headNode = FindHead(current->name);
	sNode *srcNode = (headNode) ? headNode : linkNode;

	// terminal node
	if (current->child == NULL)
	{
		if (srcNode)
		{
			parentNode->out.push_back(srcNode);
			// ��ũ�� �Ѿ ���� �Ųٷ� Ÿ�� ���� ���� �Ѵ�.
			// �׷��� in ���� �߰����� ����.
		}
	}
	else
	{ // none terminal node
		if (srcNode && !headNode)
		{
			dbg::ErrLog("duplicate node id = [%s] \n", current->name);
			return NULL;
		}
	}

	sNode *newNode = srcNode;

	// parent setting node
	if (string("parent") == current->name)
	{
		if (parentNode)
		{
			// node name�� ������ ��� �Ӽ����� �����Ѵ�.
			parentNode->tag = current->tag;
			parentNode->delay = current->delay;
			parentNode->noProc = current->noProc;
			parentNode->isAuto = current->isAuto;
			parentNode->isSideMenu = current->isSideMenu;
			parentNode->key = current->key;
			parentNode->noUpperTraverse = current->noUpperTraverse;
			parentNode->isEnterChild = current->isEnterChild;
			parentNode->isNoMenu = current->isNoMenu;
			parentNode->isSideSubmenu = current->isSideSubmenu;
			parentNode->isSceneIdInherit = current->isSceneIdInherit;
			parentNode->isSceneIdChildInherit = current->isSceneIdChildInherit;
			parentNode->isCircularMenu = current->isCircularMenu;

			parentNode->sceneId = (current->sceneId == 0) ? m_sceneIdGen++ : current->sceneId;
		}
	} 
	else if (!newNode)
	{
		newNode = new sNode;
		newNode->id = current->name;
		newNode->tag = current->tag;
		newNode->delay = current->delay;
		newNode->noProc = current->noProc;
		newNode->isAuto = current->isAuto;
		newNode->isSideMenu = current->isSideMenu;
		newNode->key = current->key;
		newNode->noUpperTraverse = current->noUpperTraverse;
		newNode->isEnterChild = current->isEnterChild;
		newNode->isNoMenu = current->isNoMenu;
		newNode->isSideSubmenu = current->isSideSubmenu;
		newNode->isSceneIdInherit = current->isSceneIdInherit;
		newNode->isSceneIdChildInherit = current->isSceneIdChildInherit;
		newNode->isCircularMenu = current->isCircularMenu;
		newNode->sceneId = current->sceneId;

		if (parentNode)
		{
 			if (current->isSceneIdInherit) 
				newNode->sceneId = parentNode->sceneId ;
			if (parentNode->isSceneIdChildInherit)
				newNode->sceneId = parentNode->sceneId;
		}

		if (newNode->sceneId == 0)
			newNode->sceneId = m_sceneIdGen++;

		m_nodes.push_back(newNode);
	}

	if (srcNode)
	{
		if (parentNode)
		{
			parentNode->out.push_back(srcNode);
			srcNode->in.push_back(parentNode);
		}

		build(current, current->child, srcNode);
		build(parent, current->next, parentNode);
	}
	else if (newNode)
	{
		// Link ��尡 �ƴҶ�, ��尣 ���� (Link ���� �̹� ����Ǿ� ����)
		if (parentNode)
		{
			parentNode->out.push_back(newNode);
			newNode->in.push_back(parentNode);
		}
		else
		{
			m_heads.push_back(newNode); // ��� ���
		}

		build(current, current->child, newNode);

		// �θ� sidesel �Ӽ��� ���� ���, child �� ��� side sel �Ӽ��� ������ �Ѵ�.
		// �׸��� �θ��� sidesel �Ӽ��� ���ŵȴ�.
		if (newNode && newNode->isSideSubmenu)
		{
			newNode->isSideSubmenu = false;
			for each (auto next in newNode->out)
				next->isSideSubmenu = true;
		}

		build(parent, current->next, parentNode);
	}
	else
	{
		// parent node
		build(parent, current->next, parentNode);
	}

	return newNode;
}


cGraphScript::sNode* cGraphScript::FindParent(sNode *current, const string &id)
{
	RETV(!current, NULL);
	RETV(id.empty(), NULL);

	CheckClearAllNode();
	return FindParentRec(current, id);
}

// �θ𿡼� id ��尡 �ִ��� �˻��Ѵ�. ã���� ��带 ����
cGraphScript::sNode* cGraphScript::FindParentRec(sNode *current, const string &id)
{
	RETV(!current, NULL);

	if (current->id == id)
		return current;

	for each (auto node in current->in)
	{
		if (node->check) // already visit node
			continue;
		if (node->id == id)
			return node;
		node->check = true;
		if (sNode *p = FindParentRec(node, id))
			return p;
	}

	return NULL;
}


void cGraphScript::Clear()
{
	m_root = NULL;
	m_sceneIdGen = 1;

	for each(auto p in m_nodes)
		delete p;
	m_nodes.clear();
	m_heads.clear();
	m_parser.Clear();
}


cGraphScript::sNode* cGraphScript::Find(const string &id)
{
//	CheckClearAllNode();
//	return TraverseRec(m_root, id);
	for each (auto &node in m_nodes)
	{
		if (node->id == id)
			return node;
	}
	return NULL;
}


cGraphScript::sNode* cGraphScript::FindHead(const string &id)
{
	for each (auto &node in m_heads)
	{
		if (node->id == id)
			return node;
	}
	return NULL;
}

// 
// cGraphScript::sNode* cGraphScript::Traverse(const string &id)
// {
// 	CheckClearAllNode();
// 	return TraverseRec(m_root, id);
// }
// 
// 
// cGraphScript::sNode* cGraphScript::TraverseRec(sNode *current, const string &id)
// {
// 	RETV(!current, NULL);
// 
// 	current->check = true;
// 
// 	if (current->id == id)
// 		return current;
// 
// 	for each (auto node in current->in)
// 	{
// 		if (node->check) // already visit node
// 			continue;
// 		if (node->id == id)
// 			return node;
// 		node->check = true;
// 		if (sNode *p = TraverseRec(node, id))
// 			return p;
// 	}
// 
// 	for each (auto node in current->out)
// 	{
// 		if (node->check) // already visit node
// 			continue;
// 		if (node->id == id)
// 			return node;
// 		node->check = true;
// 		if (sNode *p = TraverseRec(node, id))
// 			return p;
// 	}
// 
// 	return NULL;
// }


// from ��忡�� to ���� ���� ��Ʈ�� ������ �����Ѵ�.
bool cGraphScript::FindRoute(const string &from, const string &to, OUT vector<sNode*> &out)
{
	RETV(from == to, true);

	sNode *start = Find(from);
	RETV(!start, false);

	CheckClearAllNode();

	return FindRouteRec(start, to, out);
}


bool cGraphScript::FindRoute(sNode*current, const string &to, OUT vector<sNode*> &out)
{
	CheckClearAllNode();
	return FindRouteRec(current, to, out);
}


// ���� ª�� ��θ� �����Ѵ�.
bool cGraphScript::FindRouteRec(sNode*current, const string &id, OUT vector<sNode*> &out)
{
	RETV(!current, false);

	current->check = true;

	vector<sNode*> path1, path2;
	path1.push_back(current);
	path2.push_back(current);
	//out.push_back(current);

	if (current->id == id)
		return true;

	if (!current->noUpperTraverse)
	{
		for each (auto node in current->in)
		{
			if (node->check) // already visit node
				continue;
			if (node->isAuto) // �����ð��� ������ ������ ����, ���ã�⿡�� ���ܽ�Ų��.
				continue;

			if (node->id == id)
			{
				path1.push_back(node);
				break;// return true;
			}
			node->check = true;
			if (FindRouteRec(node, id, path1))
				break;//return true;
		}
	}

	for each (auto node in current->out)
	{
		if (node->check) // already visit node
			continue;
		if (node->isAuto) // �����ð��� ������ ������ ����, ���ã�⿡�� ���ܽ�Ų��.
			continue;

		if (node->id == id)
		{
			path2.push_back(node);
			break;// return true;
		}
		node->check = true;
		if (FindRouteRec(node, id, path2))
			break;// return true;
	}


	if (!path1.empty() && !path2.empty())
	{
		// �� ��� ��� �������� ���� �ߴٸ�, 
		if ((path1.back()->id == id) && (path2.back()->id == id))
		{
			// �� ª�� ��θ� �����Ѵ�.
			if (path1.size() <= path2.size())
				std::copy(path1.begin(), path1.end(), std::back_inserter(out));
			else
				std::copy(path2.begin(), path2.end(), std::back_inserter(out));
		}
		else if (path1.back()->id == id) // ��� 1�� �������� �����ߴٸ�
		{
			std::copy(path1.begin(), path1.end(), std::back_inserter(out));
		}
		else if(path2.back()->id == id) // ��� 2�� �������� �����ߴٸ�
		{
			std::copy(path2.begin(), path2.end(), std::back_inserter(out));
		}
	}
	else if (!path1.empty())
	{
		if (path1.back()->id == id) // �������� �����ߴٸ�.
			std::copy(path1.begin(), path1.end(), std::back_inserter(out));
	}
	else if (!path2.empty())
	{
		if (path2.back()->id == id) // �������� �����ߴٸ�.
			std::copy(path2.begin(), path2.end(), std::back_inserter(out));
	}

	// �������� �����ߴٸ� true�� �����Ѵ�.
	if (!out.empty() && (out.back()->id == id))
		return true;

	return false;
}


// �׷��� �ڷᱸ���� ��ȸ�ϱ� ����, üũ�� �����ϰ� ��ȸ�Ѵ�.
// �ߺ� ��ȸ�� �������� ���δ�.
void cGraphScript::CheckClearAllNode()
{
	for each(auto p in m_nodes)
		p->check = false;
}