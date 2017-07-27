
#include "../stdafx.h"
#include "window2.h"

using namespace framework;
using namespace graphic;

cWindow2::cWindow2(cSprite &sprite, const int id, const string &name) : // name="window"
	cSprite2(sprite, id, name)
,	m_isVisible(true)
{
}

cWindow2::~cWindow2()
{
}


bool cWindow2::Init(cRenderer &renderer)
{
	// �Ļ�Ŭ�������� �����Ѵ�.
	return true;
}


bool cWindow2::Update(const float deltaSeconds)
{
	return cSprite2::Update(deltaSeconds);
}


void cWindow2::Render(cRenderer &renderer, const Matrix44 &parentTm)
{
	cSprite2::Render(renderer, parentTm);
}


// �޼��� �̺�Ʈ ó��.
bool cWindow2::MessageProc( UINT message, WPARAM wParam, LPARAM lParam)
{
	// child window message process
	BOOST_REVERSE_FOREACH (auto &node, m_children)
	{
		if (cWindow2 *childWnd = dynamic_cast<cWindow2*>(node))
		{
			if (childWnd->MessageProc(message, wParam, lParam))
				break;
		}
	}
	
	return false;
}


// �̺�Ʈ �ڵ鷯 ȣ��
bool cWindow2::DispatchEvent( cEvent &event )
{
	cEventHandler::SearchEventTable(event);
	if (!event.IsSkip() && m_parent) // �̺�Ʈ�� ��ŵ���� �ʾҴٸ� �θ�� �ö󰡼� �����Ѵ�.
	{
		if (cWindow2 *parent = dynamic_cast<cWindow2*>(m_parent))
			parent->DispatchEvent(event);
	}
	return true;
}
