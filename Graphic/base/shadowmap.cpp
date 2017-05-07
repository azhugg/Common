
#include "stdafx.h"
#include "shadowmap.h"

using namespace graphic;


cShadowMap::cShadowMap()
{
}

cShadowMap::~cShadowMap()
{
}


// �׸��� Ŭ���� ����.
bool cShadowMap::Create(cRenderer &renderer, const int textureWidth, const int textureHeight)
{
	return m_surface.Create(renderer, textureWidth, textureHeight, 1,
		D3DFMT_X8R8G8B8, true, D3DFMT_D24X8, true);
}


// �׸��� �ؽ��ĸ� ������Ʈ �Ѵ�.
//void cShadowMap::UpdateShadow(cRenderer &renderer, cNode &node)
//{
//	RET(!m_surface.IsLoaded());
//
//	const Vector3 pos = node.GetTransform().GetPosition();
//
//	// ���� �������� ���� �׸��� ������ �ʿ��� ������ ���´�.
//	Vector3 lightPos;
//	Matrix44 view, proj, tt;
//	cLightManager::Get()->GetMainLight().GetShadowMatrix(
//		pos, lightPos, view, proj, tt);
//
//	if (node.m_shader)
//	{
//		node.m_shader->SetMatrix("g_mView", view);
//		node.m_shader->SetMatrix("g_mProj", proj);
//	}
//
//	Begin(renderer);
//	node.RenderShadow(renderer, view*proj, lightPos, Vector3(0, -1, 0), Matrix44::Identity);
//	End();
//}


void cShadowMap::Bind(cShader &shader, const string &key)
{
	m_surface.Bind(shader, key);
}


void cShadowMap::Begin(cRenderer &renderer)
{
	m_surface.Begin();
	renderer.GetDevice()->Clear(0, NULL
		, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
		, 0x00000000, 1.0f, 0L);
}


void cShadowMap::End()
{
	m_surface.End();
}


// �׸��� �� ���. (������)
void cShadowMap::RenderShadowMap(cRenderer &renderer)
{
	m_surface.Render(renderer);
}


void cShadowMap::LostDevice()
{
	m_surface.LostDevice();
}


void cShadowMap::ResetDevice(graphic::cRenderer &renderer)
{
	m_surface.ResetDevice(renderer);
}
