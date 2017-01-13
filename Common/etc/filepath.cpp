
#include "stdafx.h"
#include "FilePath.h"
#include <shlwapi.h> // �� ��� ���Ͽ� FilePath�� ���õ� �Լ����� ����. �� �̿��غ���.
#include <io.h>
#include <Shellapi.h>
#pragma comment(lib, "shlwapi")
//#include <sys/types.h>  
#include <sys/stat.h>


namespace common {

	bool CompareExtendName(const char *srcFileName, int srcStringMaxLength, const char *compareExtendName);

}


//------------------------------------------------------------------------
// fileName���� �����̸��� Ȯ���ڸ� ������ ������ ��θ� �����Ѵ�.
// �������� '\' ���ڴ� ����.
//------------------------------------------------------------------------
string common::GetFilePathExceptFileName(const std::string &fileName)
{
	char srcFileName[ MAX_PATH];
	strcpy_s(srcFileName, MAX_PATH, fileName.c_str() );
	PathRemoveFileSpecA( srcFileName );
	return srcFileName;
}


/**
 @brief  fileName���� Ȯ���ڸ� �����Ѵ�.
 */
string common::GetFileExt(const string &fileName)
{
	char *ext = PathFindExtensionA(fileName.c_str());
	return ext;
}

string common::RemoveFileExt(const string &fileName)
{
	char tmp[MAX_PATH] = { NULL, };
	strcpy_s(tmp, fileName.c_str());
	PathRemoveExtensionA(tmp);
	return tmp;
}


// ���丮 ��θ� ������ �� ���� �̸��� �����Ѵ�. (������� �� �̿�).
string common::GetFullFileName(const string &fileName)
{
	char dstFileName[ MAX_PATH] = {NULL, };

	if (IsRelativePath(fileName))
	{
		char curDir[ MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, curDir);
		const string path = string(curDir) + "/" + fileName;
		GetFullPathNameA(path.c_str(), MAX_PATH, dstFileName, NULL);
	}
	else
	{
		GetFullPathNameA(fileName.c_str(), MAX_PATH, dstFileName, NULL);
		return dstFileName;
	}

	return dstFileName;
}


/**
 @brief fileName�� ���丮 ��θ� ������ �����̸��� Ȯ���ڸ� �����Ѵ�.
 */
string common::GetFileName(const string &fileName)
{
	char srcFileName[ MAX_PATH];
	strcpy_s(srcFileName, MAX_PATH, fileName.c_str() );
	char *name = PathFindFileNameA(srcFileName);
	return name;
}


/**
 @brief fileName�� ���丮 ���, Ȯ���ڸ� ������ �����̸��� �����Ѵ�.
 */
string common::GetFileNameExceptExt(const string &fileName)
{
	char srcFileName[ MAX_PATH];
	strcpy_s(srcFileName, MAX_PATH, fileName.c_str() );
	char *name = PathFindFileNameA(srcFileName);
	PathRemoveExtensionA(name);
	return name;
}


// pathFrom��ο��� pathTo ����� ������ �����ϱ� ���� ����θ� �����Ѵ�.
// ex) 
// pathFrom : c:/project/media,  pathTo : c:/project/media/terrain/file.txt
// result = ./terrain/file.txt
string common::RelativePathTo(const string &pathFrom, const string &pathTo)
{
	char szOut[ MAX_PATH];

	const BOOL result = PathRelativePathToA(szOut,
		pathFrom.c_str(), FILE_ATTRIBUTE_DIRECTORY,
		pathTo.c_str(), FILE_ATTRIBUTE_NORMAL);

	return szOut;
}
wstring common::RelativePathToW(const wstring &pathFrom, const wstring &pathTo)
{
	wchar_t szOut[MAX_PATH];

	const BOOL result = PathRelativePathTo(szOut,
		pathFrom.c_str(), FILE_ATTRIBUTE_DIRECTORY,
		pathTo.c_str(), FILE_ATTRIBUTE_NORMAL);

	return szOut;
}


// ��� ����̸� true�� �����Ѵ�.
bool common::IsRelativePath(const string &path)
{
	return PathIsRelativeA(path.c_str())? true : false;
}


// ./dir1/dir2/file.ext  ==>  dir1/dir2/file.ext
string common::DeleteCurrentPath(const string &fileName)
{
	const int pos = fileName.find(".\\");
	if (pos == 0)
	{
		return DeleteCurrentPath(fileName.substr(2));
	}

	return fileName;
}


__int64  common::FileSize(const string &fileName)
{
	struct __stat64 buf;
	if (_stat64(fileName.c_str(), &buf) != 0)
		return -1; // error, could use errno to find out more
	return buf.st_size;
}


//-----------------------------------------------------------------------------//
// searchPath������ findExt Ȯ���� ����Ʈ�� ���Ե� ������ out�� �����Ѵ�.
//
// searchPath: Ž���ϰ��� �ϴ� ���丮 ���
//		- �������� / �־���Ѵ�.
// findExt: ã���� �ϴ� Ȯ����, 2���̻� �����Ҽ��ְ� �ϱ����ؼ� ����Ʈ �ڷ����°� �Ǿ���.
// out: ��ġ�ϴ� Ȯ���ڸ� ���� �����̸��� �����Ѵ�.
//-----------------------------------------------------------------------------//
bool common::CollectFiles( const list<string> &findExt, const string &searchPath, OUT list<string> &out)
{
	WIN32_FIND_DATAA fd;
	const string searchDir = searchPath + "*.*";
	HANDLE hFind = FindFirstFileA(searchDir.c_str(), &fd);

	while (1)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ((string(".") != fd.cFileName) && (string("..") != fd.cFileName))
			{
				CollectFiles( findExt, searchPath + string(fd.cFileName) + "/", out ); 
			}
		}
		else if (fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			const string fileName = fd.cFileName;

			if (findExt.empty())
			{
				out.push_back(searchPath + fileName);
			}
			else
			{
				auto it = findExt.begin();
				while (findExt.end() != it)
				{
					if (CompareExtendName(fileName.c_str() , (int)fileName.length(), it->c_str()))
					{
						out.push_back( searchPath + fileName );
						break;
					}
					++it;
				}
			}
		}

		if (!FindNextFileA(hFind, &fd))
			break;
	}

	FindClose(hFind);
	
	return true;
}


// ���ϸ�� ��¥ ������ ������ �����Ѵ�.
bool CollectFilesRaw(const list<string> &findExt, const string &searchPath, 
	OUT list<std::pair<FILETIME, string>> &out)
{
	WIN32_FIND_DATAA fd;
	const string searchDir = searchPath + "*.*";
	HANDLE hFind = FindFirstFileA(searchDir.c_str(), &fd);

	while (1)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((string(".") != fd.cFileName) && (string("..") != fd.cFileName))
			{
				CollectFilesRaw(findExt, searchPath + string(fd.cFileName) + "/", out);
			}
		}
		else if (fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			const string fileName = fd.cFileName;

			if (findExt.empty())
			{
				//out.push_back(searchPath + fileName);
				out.push_back(std::pair<FILETIME, string>(fd.ftLastWriteTime, searchPath + fileName));
			}
			else
			{
				auto it = findExt.begin();
				while (findExt.end() != it)
				{
					if (common::CompareExtendName(fileName.c_str(), (int)fileName.length(), it->c_str()))
					{
						out.push_back(std::pair<FILETIME, string>(fd.ftLastWriteTime, searchPath + fileName));
						break;
					}
					++it;
				}
			}
		}

		if (!FindNextFileA(hFind, &fd))
			break;
	}

	FindClose(hFind);
	return true;
}


//-----------------------------------------------------------------------------//
// searchPath������ findExt Ȯ���� ����Ʈ�� ���Ե� ������ out�� �����Ѵ�.
//
// searchPath: Ž���ϰ��� �ϴ� ���丮 ���
//		- �������� / �־���Ѵ�.
// findExt: ã���� �ϴ� Ȯ����, 2���̻� �����Ҽ��ְ� �ϱ����ؼ� ����Ʈ �ڷ����°� �Ǿ���.
// out: ��ġ�ϴ� Ȯ���ڸ� ���� �����̸��� �����Ѵ�.
// flag : 0 = �ֱ� ������ ��¥�� �������� �����Ѵ�.
//-----------------------------------------------------------------------------//
bool common::CollectFilesOrdered(const list<string> &findExt, const string &searchPath, OUT list<string> &out, 
	const int flags) // flags=0
{
	using std::pair;

	list< pair<FILETIME, string>> files;
	CollectFilesRaw(findExt, searchPath, files);

	// �ֱ� ������ ��¥ ������� ����.
	// ���ϰ� ������ ���� ����, ������ �׻� ���� ������ �Ѵ�.
	if (flags == 0)
	{
		files.sort(  
			[](const pair<FILETIME, string> &a, const pair<FILETIME, string> &b)
			{ 
				return CompareFileTime(&a.first, &b.first) > 0; 
			} 
		);
	}

	for each (auto &it in files)
		out.push_back(it.second);

	return true;
}


//------------------------------------------------------------------------
// srcFileName�� Ȯ���ڿ� compareExtendName �̸��� ���ٸ� true�� �����Ѵ�.
// Ȯ���ڴ� srcFileName ������ '.'�� ���� ������ �̴�.
//------------------------------------------------------------------------
bool common::CompareExtendName(const char *srcFileName, const int srcStringMaxLength, const char *compareExtendName)
{
	const int len = (int)strnlen_s(srcFileName, srcStringMaxLength);
	if (len <= 0)
		return FALSE;

	int count = 0;
	char temp[5];
	for (int i=0; i < len && i < 4; ++i)
	{
		const char c = srcFileName[ len-i-1];
		if ('.' == c)
		{
			break;
		}
		else
		{
			temp[ count++] = c;
		}
	}
	temp[ count] = NULL;

	char extendName[5];
	for (int i=0; i < count; ++i)
		extendName[ i] = temp[ count-i-1];
	extendName[ count] = NULL;

	if (!strcmp(extendName, compareExtendName))
	{
		return true;
	}

	return false;
}


// searchPath ���丮 �ȿ��� findName �� �����̸��� ���� ������ �ִٸ� �ش� ��θ�
// out �� �����ϰ� true �� �����Ѵ�.
bool common::FindFile( const string &findName, const string &searchPath, string &out  )
{
	WIN32_FIND_DATAA fd;
	const string searchDir = searchPath + "*.*";
	HANDLE hFind = FindFirstFileA(searchDir.c_str(), &fd);

	while (1)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ((string(".") != fd.cFileName) && (string("..") != fd.cFileName))
			{
				if (FindFile( findName, searchPath + string(fd.cFileName) + "/", out ))
					break;
			}
		}
		else if (fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			string fileName = fd.cFileName;
			//dbg::Log("%s\n", fd.cFileName);
			if (lowerCase(fileName) == lowerCase(GetFileName(findName)))
			{
			//if (fileName == GetFileName(findName))
			//{
				out = searchPath + GetFileName(findName);
				break;
			}
		}

		if (!FindNextFileA(hFind, &fd))
			break;
	}

	FindClose(hFind);

	return !out.empty();
}


bool common::IsFileExist(const string &fileName)
{
	return _access_s(fileName.c_str(), 0) == 0;
}


// Create Folder Tree,  from fileList
// fileList : fileName list
// Must Delete return value by DeleteFolderNode()
//
// root - child1
//					- child1-1
//			- child2
//					- child2-1
//
common::sFolderNode* common::CreateFolderNode(const list<string> &fileList)
{
	sFolderNode *rootNode = new sFolderNode;

	for each (auto &str in fileList)
	{
		vector<string> strs;
		common::tokenizer(str, "/", ".", strs);

		sFolderNode *node = rootNode;
		for (u_int i = 0; i < strs.size(); ++i)
		{
			if (i == (strs.size() - 1)) // Last String Is FileName, then Ignored
				continue;

			const string name = strs[i];
			auto it = node->children.find(name);
			if (node->children.end() == it)
			{
				sFolderNode *t = new sFolderNode;
				node->children[name] = t;
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


void common::DeleteFolderNode(sFolderNode *node)
{
	RET(!node);
	for each (auto &child in node->children)
		DeleteFolderNode(child.second);
	delete node;
}


// https://msdn.microsoft.com/ko-kr/library/windows/desktop/bb759795(v=vs.85).aspx
// File Copy, Delete, Rename, Move Operation
// func : FO_COPY, FO_DELETE, FO_MOVE, FO_RENAME
int common::FileOperationFunc(unsigned int func, const string &to, const string &from)
{
	char dst[MAX_PATH];
	strcpy_s(dst, to.c_str());
	dst[to.size() + 1] = NULL; // double NULL

	char src[MAX_PATH];
	strcpy_s(src, from.c_str());
	src[from.size() + 1] = NULL; // double NULL

	SHFILEOPSTRUCTA s;
	ZeroMemory(&s, sizeof(s));
	s.hwnd = NULL;
	s.wFunc = func;
	s.fFlags = FOF_NO_UI;
	s.pTo = dst;
	s.pFrom = src;
	s.lpszProgressTitle = NULL;
	return SHFileOperationA(&s);
}
