
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


cGraphScript::sNode* cGraphScript::build(sParseTree *parent, sParseTree *current, sNode *parentNode)
{
	RETV(!current, NULL);

	if (current->attrs["delay"].empty())
		current->attrs["delay"] = "0.5"; // default

	sNode *linkNode = FindParent(parentNode, current->attrs["id"]);
	sNode *headNode = FindHead(current->attrs["id"]);
	sNode *srcNode = (headNode) ? headNode : linkNode;

	if (current->child && srcNode && !headNode)
	{
		dbg::ErrLog("duplicate node id = [%s] \n", current->attrs["id"].c_str());
		return NULL;
	}

	sNode *newNode = srcNode;

	// parent setting node
	if (string("parent") == current->attrs["id"])
	{
		if (parentNode)
		{
			// node name�� ������ ��� �Ӽ����� �����Ѵ�.
			const string tmpId = parentNode->attrs["id"];
			parentNode->attrs = current->attrs;
			parentNode->attrs["id"] = tmpId;
			parentNode->sceneId = (current->attrs["sceneid"].empty())? m_sceneIdGen++ : atoi(current->attrs["sceneid"].c_str());
		}
	} 
	else if (!newNode)
	{
		newNode = new sNode;
		newNode->attrs = current->attrs;
		newNode->sceneId = atoi(current->attrs["sceneid"].c_str());

		if (parentNode)
		{
 			if (current->attrs["sceneid_inherit"] == "1") 
				newNode->sceneId = parentNode->sceneId ;
			if (parentNode->attrs["sceneid_child_inherit"] == "1")
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
		if (newNode && (newNode->attrs["sidesel"] == "1"))
		{
			newNode->attrs["sidesel"] = "0";
			for each (auto next in newNode->out)
				next->attrs["sidesel"] = "1";
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

	if (current->attrs["id"] == id)
		return current;

	for each (auto node in current->in)
	{
		if (node->check) // already visit node
			continue;
		if (node->attrs["id"] == id)
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
	for each (auto &node in m_nodes)
	{
		if (node->attrs["id"] == id)
			return node;
	}
	return NULL;
}


// current�� �߽����� ������ ID�� ���� ��带 �����Ѵ�.
cGraphScript::sNode* cGraphScript::Find(sNode*current, const string &id)
{
	if (!current)
		return Find(id);

	vector<sNode*> path;
	FindRoute(current, id, path);
	if (path.empty())
		return NULL;

	return path.back();
}


cGraphScript::sNode* cGraphScript::FindHead(const string &id)
{
	for each (auto &node in m_heads)
	{
		if (node->attrs["id"] == id)
			return node;
	}
	return NULL;
}


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

	if (current->attrs["id"] == id)
		return true;

	if (current->attrs["noup"] != "1")
	{
		for each (auto node in current->in)
		{
			if (node->check) // already visit node
				continue;
			if (node->attrs["auto"] == "1") // �����ð��� ������ ������ ����, ���ã�⿡�� ���ܽ�Ų��.
				continue;

			if (node->attrs["id"] == id)
			{
				path1.push_back(node);
				break;
			}
			node->check = true;
			if (FindRouteRec(node, id, path1))
				break;
		}
	}

	for each (auto node in current->out)
	{
		if (node->check) // already visit node
			continue;
		if (node->attrs["auto"] == "1") // �����ð��� ������ ������ ����, ���ã�⿡�� ���ܽ�Ų��.
			continue;

		if (node->attrs["id"] == id)
		{
			path2.push_back(node);
			break;
		}
		node->check = true;
		if (FindRouteRec(node, id, path2))
			break;
	}


	if (!path1.empty() && !path2.empty())
	{
		// �� ��� ��� �������� ���� �ߴٸ�, 
		if ((path1.back()->attrs["id"] == id) && (path2.back()->attrs["id"] == id))
		{
			// �� ª�� ��θ� �����Ѵ�.
			if (path1.size() <= path2.size())
				std::copy(path1.begin(), path1.end(), std::back_inserter(out));
			else
				std::copy(path2.begin(), path2.end(), std::back_inserter(out));
		}
		else if (path1.back()->attrs["id"] == id) // ��� 1�� �������� �����ߴٸ�
		{
			std::copy(path1.begin(), path1.end(), std::back_inserter(out));
		}
		else if(path2.back()->attrs["id"] == id) // ��� 2�� �������� �����ߴٸ�
		{
			std::copy(path2.begin(), path2.end(), std::back_inserter(out));
		}
	}
	else if (!path1.empty())
	{
		if (path1.back()->attrs["id"] == id) // �������� �����ߴٸ�.
			std::copy(path1.begin(), path1.end(), std::back_inserter(out));
	}
	else if (!path2.empty())
	{
		if (path2.back()->attrs["id"] == id) // �������� �����ߴٸ�.
			std::copy(path2.begin(), path2.end(), std::back_inserter(out));
	}

	// �������� �����ߴٸ� true�� �����Ѵ�.
	if (!out.empty() && (out.back()->attrs["id"] == id))
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


void PrintGraphSub(std::ofstream &ofs, cGraphScript::sNode *current, const int tab)
{
	using namespace std;
	RET(!current);

	for (int i = 0; i < tab; ++i)
		ofs << "\t";
	ofs << current->attrs["id"] << endl;

	RET(current->check);
	
	current->check = true;
	for each (auto &out in current->out)
		PrintGraphSub(ofs, out, tab + 1);
}


void cGraphScript::PrintGraph(const string &rootName)
{
	sNode *root = FindHead(rootName);
	RET(!root);

	CheckClearAllNode();

	std::ofstream ofs("print_graph.txt");
	if (ofs.is_open())
		PrintGraphSub(ofs, root, 0);
}
