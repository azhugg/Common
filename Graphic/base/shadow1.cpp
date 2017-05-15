
#include "stdafx.h"
#include "shadow1.h"

using namespace graphic;


cShadow1::cShadow1()
{
}

cShadow1::~cShadow1()
{
}


// �׸��� Ŭ���� ����.
bool cShadow1::Create(cRenderer &renderer, const int textureWidth, const int textureHeight)
{
	return m_surface.Create(renderer, textureWidth, textureHeight, 1,
		D3DFMT_X8R8G8B8, true, D3DFMT_D24X8, true);
}


// �׸��� �ؽ��ĸ� ������Ʈ �Ѵ�.
void cShadow1::UpdateShadow(cRenderer &renderer, cNode &node)
{
	RET(!m_surface.IsLoaded());

	const Vector3 pos = node.GetTransform().GetPosition();

	// ���� �������� ���� �׸��� ������ �ʿ��� ������ ���´�.
	Vector3 lightPos;
	Matrix44 view, proj, tt;
	cLightManager::Get()->GetMainLight().GetShadowMatrix(
		pos, lightPos, view, proj, tt );

	if (node.m_shader)
	{
		node.m_shader->SetMatrix("g_mView", view);
		node.m_shader->SetMatrix("g_mProj", proj);
	}

	Begin(renderer);
	node.RenderShadow(renderer, view*proj, lightPos, Vector3(0,-1,0), Matrix44::Identity);
	End(renderer);
}


void cShadow1::UpdateShadow(cRenderer &renderer, cModel2 &model)
{
	RET(!m_surface.IsLoaded());

	const Vector3 pos = model.m_tm.GetPosition();

	// ���� �������� ���� �׸��� ������ �ʿ��� ������ ���´�.
	Vector3 lightPos;
	Matrix44 view, proj, tt;
	cLightManager::Get()->GetMainLight().GetShadowMatrix(
		pos, lightPos, view, proj, tt);

	if (model.m_shader)
	{
		model.m_shader->SetMatrix("g_mView", view);
		model.m_shader->SetMatrix("g_mProj", proj);
	}

	Begin(renderer);
	model.RenderShader(renderer);
	End(renderer);
}


void cShadow1::Bind(cShader &shader, const string &key)
{

}


void cShadow1::Begin(cRenderer &renderer)
{
	m_surface.Begin(renderer);
	renderer.GetDevice()->Clear(0, NULL
		, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
		, 0x00000000, 1.0f, 0L);
}


void cShadow1::End(cRenderer &renderer)
{
	m_surface.End(renderer);
}


// �׸��� �� ���. (������)
void cShadow1::RenderShadowMap(cRenderer &renderer)
{
	m_surface.Render(renderer);
}


void cShadow1::LostDevice()
{
	m_surface.LostDevice();
}


void cShadow1::ResetDevice(graphic::cRenderer &renderer)
{
	m_surface.ResetDevice(renderer);
}
