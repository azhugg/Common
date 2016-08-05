#include "stdafx.h"
#include "matchscript2.h"
#include "matchprocessor.h"

using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;
using namespace std;


cMatchScript2::cMatchScript2()
 	: m_matchType(0)
 	, m_treeId(0)
	, m_cloneLinkId(0)
{
}

cMatchScript2::~cMatchScript2()
{
	Clear();
}


void cMatchScript2::build(sParseTree *parent, sParseTree *prev, sParseTree *current)
{
	RET(!current);

	vector<string> attribs;
	const bool isSymbolCode = buildAttributes(current, current->line, attribs) > 0;

	// check tree label node link
	// only link node,  paren==null is head node
	if ((current->type == 0) && !attribs.empty() && parent)
	{
		if (const sParseTree *linkNode = FindTreeLabel(attribs[0])) // ��ũ��� �ϰ��.
		{
			// �߰��� �ɺ��� �Ľ�Ʈ���� �����Ѵ�. ���尡 ������, �߰��� �ɺ����̺��� �����Ѵ�.
			// �Ľ�Ʈ���� ��ũ�� �ƴ�, ���� ���� �Ǵ� ���̴�.
			if (isSymbolCode)
			{
				sParseTree *node = m_parser.cloneTree(linkNode);
				node->next = current->next;
				SAFE_DELETE(current);
				if (prev)
					prev->next = node;
				else
					parent->child = node;

				setTreeAttribute(node, attribs);

				// ���� �߰��� Ʈ����, ���� ��ũ Ʈ���� �������� ���� �̸��� �ٲ۴�.
				strcat_s(node->name, "_clone");

				build(node, NULL, node->child);
				m_parser.RemoveSymbolTable(); // �ڽ� ������ �ɺ����̺��� �����ϰ�, ���Ŀ��� �����Ѵ�.

				build(parent, node, node->next);
				return;
			}
			else
			{
				// ��ũ����� Ŭ���� ����� �߰��Ѵ�.
				sParseTree *node = new sParseTree;
				memcpy(node, linkNode, sizeof(sParseTree));
				node->next = current->next;
				SAFE_DELETE(current);
				if (prev)
					prev->next = node;
				else
					parent->child = node;

				build(parent, node, node->next);
				return;
			}
		}
	}

	setTreeAttribute(current, attribs);

	build(current, NULL, current->child);
	build(parent, current, current->next);
}


// tok1, tok2, tok3, tok4, tok5 ...  �и� 
// node->str = tok1
// tok2~5 : roi x,y,w,h
// { tok = tok }
// return value : 0 = none
//						   1,2,3... = add symbol table, symbolCode
int cMatchScript2::buildAttributes(const sParseTree *node, const string &str, vector<string> &attributes)
{
	int reval = 0;

 	m_parser.m_lineStr = (char*)str.c_str();
	while (1)
	{
		const char *pid = m_parser.id();
		const char *pnum = (!*pid) ? m_parser.number() : NULL;

		if (*pid)
		{
			const string symb = m_parser.GetSymbol(pid);
			const int symbType = m_parser.GetSymbolType(pid);
			if (symbType == 0) // string type
			{
				if (symb.empty())
				{
					attributes.push_back(pid);
				}
				else
				{
					char *old = m_parser.m_lineStr;
					reval += buildAttributes(node, symb.c_str(), attributes);
					m_parser.m_lineStr = old;
				}
			}
			else if (symbType == 1)
			{
				// string table type,
				// setTreeAttribute() ���� ó���Ѵ�.
				attributes.push_back(pid);
			}
		}
		else if (*pnum)
		{
			attributes.push_back(pnum);
		}
		else if (m_parser.m_lineStr[0] == '{')
		{
			// parsing, { id = id, id = id } 
			if (m_parser.assigned_list(node))
			{
				// �ɺ����̺� Ʈ�� �߰�
				reval += 1;
			}
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

	return reval;
}


// threshold_0.9 
// templatematch, featurematch
void cMatchScript2::setTreeAttribute(sParseTree *node, vector<string> &attribs)
{
	node->id = m_treeId++;

	if (attribs.empty())
		return;

	// check threshold
	for (int i = 0; i < (int)attribs.size(); ++i)
	{
		const int pos = attribs[i].find("threshold_");
		if (string::npos != pos)
		{
			// threshold �� ����
			const int valPos = attribs[i].find("_");
			const float threshold = (float)atof(attribs[i].substr(valPos + 1).c_str());
			node->threshold = threshold;

			// remove threshold attribute
			std::rotate(attribs.begin() + i, attribs.begin() + i + 1, attribs.end());
			attribs.pop_back();
			break;
		}
	}

	// check match type
	node->matchType = -1; // defautl value is -1
	for (int i = 0; i < (int)attribs.size(); ++i)
	{
		const int pos1 = attribs[i].find("featurematch");
		const int pos2 = attribs[i].find("templatematch");
		const int pos3 = attribs[i].find("ocrmatch");
		if ((string::npos != pos1) || (string::npos != pos2) || (string::npos != pos3))
		{
			if (string::npos != pos1)
				node->matchType = 1;
			if (string::npos != pos2)
				node->matchType = 0;
			if (string::npos != pos3)
				node->matchType = 2;

			// remove threshold attribute
			std::rotate(attribs.begin() + i, attribs.begin() + i + 1, attribs.end());
			attribs.pop_back();
			break;
		}
	}

	// check scalar
	// scalar_255_0_255_1.5
	for (int i = 0; i < (int)attribs.size(); ++i)
	{
		const int pos = attribs[i].find("scalar_");
		if (string::npos != pos)
		{
			// threshold �� ����
			const int valPos = attribs[i].find("_");
			sscanf(attribs[i].c_str(), "scalar_%d_%d_%d_%f", &node->scalar[0], &node->scalar[1], &node->scalar[2], &node->scale);

			// remove threshold attribute
			std::rotate(attribs.begin() + i, attribs.begin() + i + 1, attribs.end());
			attribs.pop_back();
			break;
		}
	}

	// check hsv
	// hsv_40_40_255_120_255_255
	for (int i = 0; i < (int)attribs.size(); ++i)
	{
		const int pos = attribs[i].find("hsv_");
		if (string::npos != pos)
		{
			// hsv �� ����
			const int valPos = attribs[i].find("_");
			sscanf(attribs[i].c_str(), "hsv_%d_%d_%d_%d_%d_%d", &node->hsv[0], &node->hsv[1], &node->hsv[2], 
				&node->hsv[3], &node->hsv[4], &node->hsv[5] );

			// remove threshold attribute
			std::rotate(attribs.begin() + i, attribs.begin() + i + 1, attribs.end());
			attribs.pop_back();
			break;
		}
	}


	// string table
	for (int i = 0; i < (int)attribs.size(); ++i)
	{
		if (vector<string> *table = m_parser.GetSymbol2(attribs[i]))
		{
			// string table ����
			node->table = table;

			// remove threshold attribute
			std::rotate(attribs.begin() + i, attribs.begin() + i + 1, attribs.end());
			attribs.pop_back();
			break;
		}
	}

	strcpy_s(node->name, attribs[0].c_str());

	if (node->type == 0) // tree
	{
		if (attribs.size() >= 5)
		{
			for (int i = 1; i < 5; ++i)
				node->roi[i - 1] = atoi(attribs[i].c_str());
		}
		if (attribs.size() >= 6)
		{
			if (attribs[5] == "+")
				node->isRelation = true;
		}
	}
	else if (node->type == 1) // exec
	{
		if (attribs.size() >= 2)
		{
			strcat_s(node->name, " ");
			strcat_s(node->name, attribs[1].c_str());
		}
	}

}


// node �� ��Ī�� �����ߴٸ�, ������ ��ġ(matchLoc)�� �̹����� ������ �����Ѵ�.
void cMatchScript2::GetCloneMatchingArea(const Mat &input, const string &inputName, const int inputImageId, 
	sParseTree *node, OUT cv::Mat *out)
{
	const Mat &matScene = cMatchProcessor::Get()->loadImage(inputName);
	if (matScene.empty())
		return;

	const Mat &matObj = cMatchProcessor::Get()->loadImage(node->name);
	if (matObj.empty())
		return;

	const cv::Size csize(input.cols - matObj.cols + 1, input.rows - matObj.rows + 1);
	if ((csize.height < 0) || csize.width < 0)
		return;

	const Mat *src = &matScene;

	// channel match
	if (!node->IsEmptyBgr())
	{
		src = &cMatchProcessor::Get()->loadScalarImage(inputName, inputImageId, Scalar(node->scalar[0], node->scalar[1], node->scalar[2]), node->scale); // BGR
	}

	// hsv match
	if (!node->IsEmptyHsv())
	{
		src = &cMatchProcessor::Get()->loadHsvImage(inputName, inputImageId, Scalar(node->hsv[0], node->hsv[1], node->hsv[2]), Scalar(node->hsv[3], node->hsv[4], node->hsv[5]));
	}

	if (!src->data)
		return;// fail

	const bool isFeatureMatch = ((m_matchType == 1) && (node->matchType == -1)) || (node->matchType == 1);
	if (!isFeatureMatch) // --> templatematch
	{
		Point left_top = node->matchLoc;
		*out = (*src)(Rect(left_top.x, left_top.y, matObj.cols, matObj.rows));
	}
}


// ��ũ��Ʈ�� exec ��ɾ �����Ѵ�.
void cMatchScript2::Exec()
{
	sParseTree *node = m_parser.m_execRoot;
	while (node)
	{
		stringstream ss(node->name);
		string label, file;
		ss >> label >> file;
		if (label.empty() || file.empty())
			break;

		Mat img = imread(file.c_str());
		if (!img.data)
			break;

		const int t1 = timeGetTime();
		//ExecuteEx(label, file, img, true);
		const int t2 = timeGetTime();
		//cout << "exec(" << label << ") << " << file << " = " << m_threadArg.resultStr << ", " << t2-t1 << endl;

		node = node->next;
	}
}


const sParseTree* cMatchScript2::FindTreeLabel(const string &label) const
{
	auto it = m_treeLabelTable.find(label);
	return (m_treeLabelTable.end() != it)? it->second : NULL;
}


// ��ũ��Ʈ�� �а�, �Ľ�Ʈ���� �����Ѵ�.
bool cMatchScript2::Read(const string &fileName)
{
	Clear();

	if (!m_parser.Read(fileName))
		return false;

	// head node���� next ��ũ�� ��� �����Ѵ�. ���������� �۵��ϴ� Ʈ���� ����� ���ؼ���.
	vector<sParseTree*> headNodes; // Ʈ�� �������� ������� ó���ϱ� ���ؼ� ����
	sParseTree *node = m_parser.cloneTree(m_parser.m_treeRoot); // tree copy
	while (node)
	{
		if ('@' != node->line[0])
		{
			MessageBoxA(NULL, "Error!! ImageMatchScript LabelNode Name must start \'@\' ", "Error", MB_OK);
			return false;
		}

		m_treeLabelTable[node->line] = node;
		headNodes.push_back(node);
		sParseTree *next = node->next;
		node->next = NULL; // tree label node���� �ڽĸ� ����� ���·� ��ϵȴ�. next�� ����.
		node = next;
	}

	m_treeId = 0;
	m_cloneLinkId = 0;
	build(NULL, NULL, m_parser.m_execRoot);
	for each (auto it in headNodes)
		build(NULL, NULL, it);

  	m_isDebug = atoi(m_parser.GetSymbol("debug").c_str()) ? true : false;
  	m_matchType = atoi(m_parser.GetSymbol("matchtype").c_str());

	// update node table
	m_nodes.clear();
	for each (auto it in m_treeLabelTable)
		m_parser.collectTree(it.second, m_nodes);

	ZeroMemory(m_nodeTable, sizeof(m_nodeTable));
	for each (auto it in m_nodes)
		m_nodeTable[it->id] = it;

	return true;
}


void cMatchScript2::Clear()
{
	m_parser.Clear();

	for each (auto kv in m_nodes)
		delete kv;
	m_nodes.clear();
	m_treeLabelTable.clear();
}


// sParsetree �� result ���� �ʱ�ȭ �Ѵ�.
// �Լ��� ��������� ȣ��� ��, �ߺ� ������ �������� ���δ�.
void cMatchScript2::clearResultTree() 
{
	for each (auto it in m_nodes)
	{
		it->result = -1;
		it->processCnt = 0;
	}
}
