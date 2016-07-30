// UI ���� �������� ����ϴ� �Լ����� ���� ��ƿ��Ƽ �ڵ��.

#pragma once


//--------------------------------------------------------------------------------------------------
// Macro

// CWnd* ��ü ����
#define DELETE_WINDOW(window) \
	if (window)\
	{\
		window->DestroyWindow();\
		delete window;\
		window = NULL;\
	}



//--------------------------------------------------------------------------------------------------
// Function

void ExpandAll(CTreeCtrl &treeCtrl);

HTREEITEM FindTree( CTreeCtrl &treeCtrl, const wstring &text );

void MoveChildCtrlWindow(CWnd &parent, CWnd &wndCtrl, int cx, int cy);

std::string GetIP(CIPAddressCtrl &ipControl);
void SetIP(CIPAddressCtrl &ipControl, const string &ip);

// class cPlotInputParser;
// void ParsePlotInputStringFormat(const string &str, OUT vector<cPlotInputParser> &out);



