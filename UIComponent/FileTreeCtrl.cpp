// FileTreeCtrl.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "FileTreeCtrl.h"
#include "UIUtiltity.h"


CImageList *CFileTreeCtrl::m_imageList = NULL;
int CFileTreeCtrl::m_imageListRefCnt = 0;

// CFileTreeCtrl
CFileTreeCtrl::CFileTreeCtrl()
{
	if (!m_imageList)
	{
		SHFILEINFO sfi = { 0 }; 
		m_imageList = new CImageList();
		m_imageList->Attach((HIMAGELIST)SHGetFileInfo(L"C:\\", 0, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX));
	}

	++m_imageListRefCnt;

}

CFileTreeCtrl::~CFileTreeCtrl()
{
	--m_imageListRefCnt;
	if (m_imageListRefCnt <= 0)
	{
		SAFE_DELETE(m_imageList);
		m_imageListRefCnt = 0;
	}
}


BEGIN_MESSAGE_MAP(CFileTreeCtrl, CTreeCtrl)
END_MESSAGE_MAP()



// CFileTreeCtrl �޽��� ó�����Դϴ�.

// fileList�� ����� ���� ��� ��Ʈ������ Ʈ���� �����.
void CFileTreeCtrl::Update(const list<string> &fileList)
{
	DeleteAllItems();	

	SetImageList(m_imageList, TVSIL_NORMAL);

	sTreeNode *rootNode = GenerateTreeNode(fileList);
	GenerateTreeItem(NULL, rootNode);	
	Expand(GetRootItem(), TVE_EXPAND);

	DeleteTreeNode(rootNode);

	m_fileCount = (int)fileList.size();
}


// flag : 0 = �̸� ������� ����
//			  1 = �ֱ� ������ ������� ����
void CFileTreeCtrl::Update(const string &directoryPath, const list<string> &extList, const int flags) // flags=0
{
	list<string> files;
	
	if (flags == 0)
		common::CollectFiles(extList, directoryPath, files);
	else if (flags == 1)
		common::CollectFilesOrdered(extList, directoryPath, files);

	Update(files);
}


// searchStr ���ڿ��� ���Ե� �͸� ȭ�鿡 ����Ѵ�.
void CFileTreeCtrl::Update(const string &directoryPath, const list<string> &extList, const string &serchStr )
{
	list<string> files;
	common::CollectFiles(extList, directoryPath, files);

	if (serchStr.empty())
	{
		Update(files);
	}
	else
	{
		list<string> fileList;
		for each(auto name in files)
		{
			if (string::npos != common::GetFileName(name).find(serchStr))
				fileList.push_back(name);
		}

		Update(fileList);
	}	
}


// sTreeNode �����Ѵ�.
// ������ ���Ϻ��� ���� �ְ� �Ѵ�.
CFileTreeCtrl::sTreeNode* CFileTreeCtrl::GenerateTreeNode(const list<string> &fileList)
{
	sTreeNode *rootNode = new sTreeNode;

	for each (auto &str in fileList)
	{
		vector<string> strs;
		common::tokenizer( str, "/", ".", strs);

		sTreeNode *node = rootNode;
		for each (auto &name in strs)
		{
			auto it = node->children.find(name);
			if (node->children.end() ==  it)
			{
				sTreeNode *t = new sTreeNode;
				node->children[ name] = t;

				// Ȯ���ڰ� ���ٸ�, ������� �����Ѵ�.
				// Ȯ���ڴ� 3������ ���ų� �۾ƾ� �Ѵ�.
				const int pos = name.find_last_of('.');
 				if ((string::npos == pos) || ((int)name.size() - 4 > pos))
 				{
 					// ������ ���� ����Ʈ���� ������ �̵��Ѵ�.
					node->childrenFiles.push_front(std::pair<string, sTreeNode*>(name, t));
 				}
				else
				{
					node->childrenFiles.push_back(std::pair<string, sTreeNode*>(name, t));
				}

				node = t;
			}
			else
			{
				node = it->second;
			}
		}	
	}

	return rootNode;
}


// treenode ����.
void CFileTreeCtrl::DeleteTreeNode(sTreeNode *node)
{
	RET(!node);
	for each (auto &child in node->children)
		DeleteTreeNode(child.second);
	delete node;
}


// sTreeNode ������ TreeCtrl ��带 �����Ѵ�.
void CFileTreeCtrl::GenerateTreeItem(HTREEITEM parent, sTreeNode*node)
{
	RET(!node);

	for each (auto &child in node->childrenFiles)
	{
		const int imageIdx = child.second->children.empty()? 0 : 1;
		HTREEITEM item = InsertItem(str2wstr(child.first).c_str(), imageIdx, imageIdx, parent);
		GenerateTreeItem(item, child.second);
	}
}


// ���õ� Ʈ������� ���� ��θ� �����Ѵ�.
string CFileTreeCtrl::GetSelectFilePath(HTREEITEM item)
{
	wstring path;
	HTREEITEM curItem = item;
	while (curItem)
	{
		if (path.empty())
			path = (wstring)GetItemText(curItem);
		else
			path = (wstring)GetItemText(curItem) + L"/" + path;

		curItem = GetParentItem(curItem);
	}

	return wstr2str(path);
}


// ��� Ʈ�� ������ ��ģ��.
void CFileTreeCtrl::ExpandAll()
{
	::ExpandAll(*this);
}
