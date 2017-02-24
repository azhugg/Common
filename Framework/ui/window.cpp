
#include "stdafx.h"
#include "window.h"

using namespace framework;
using namespace graphic;

cWindow::cWindow(cSprite &sprite, const int id, const string &name) : // name="window"
	cSprite2(sprite, id, name)
,	m_isVisible(true)
{
}

cWindow::~cWindow()
{
}


bool cWindow::Init(cRenderer &renderer)
{
	// �Ļ�Ŭ�������� �����Ѵ�.
	return true;
}


bool cWindow::Update(const float deltaSeconds)
{
	return cSprite2::Update(deltaSeconds);
}


void cWindow::Render(cRenderer &renderer, const Matrix44 &parentTm)
{
	cSprite2::Render(renderer, parentTm);
}


// �޼��� �̺�Ʈ ó��.
bool cWindow::MessageProc( UINT message, WPARAM wParam, LPARAM lParam)
{
	// child window message process
	BOOST_REVERSE_FOREACH (auto &node, m_children)
	{
		if (cWindow *childWnd = dynamic_cast<cWindow*>(node))
		{
			if (childWnd->MessageProc(message, wParam, lParam))
				break;
		}
	}
	
	return false;
}


// �̺�Ʈ �ڵ鷯 ȣ��
bool cWindow::DispatchEvent( cEvent &event )
{
	cEventHandler::SearchEventTable(event);
	if (!event.IsSkip() && m_parent) // �̺�Ʈ�� ��ŵ���� �ʾҴٸ� �θ�� �ö󰡼� �����Ѵ�.
	{
		if (cWindow *parent = dynamic_cast<cWindow*>(m_parent))
			parent->DispatchEvent(event);
	}
	return true;
}
