
#include "stdafx.h"
#include "light.h"

using namespace graphic;

cLight::cLight()
{
}

cLight::~cLight()
{
}


void cLight::Init(TYPE type, 
	const Vector4 &ambient, // Vector4(0.2, 0.2, 0.2, 1)
	const Vector4 &diffuse, // Vector4(1, 1, 1, 1)
	const Vector4 &specular, // Vector4(1,1,1,1)
	const Vector3 &direction) // Vector3(0,-1,0)
{
	ZeroMemory(&m_light, sizeof(m_light));
	m_light.Type = (D3DLIGHTTYPE)type;
	m_light.Ambient = *(D3DCOLORVALUE*)&ambient;
	m_light.Diffuse = *(D3DCOLORVALUE*)&diffuse;
	m_light.Specular = *(D3DCOLORVALUE*)&specular;
	m_light.Direction = *(D3DXVECTOR3*)&direction;
}


void cLight::SetDirection( const Vector3 &direction )
{
	m_light.Direction = *(D3DXVECTOR3*)&direction;
}


void cLight::SetPosition( const Vector3 &pos )
{
	m_light.Position = *(D3DXVECTOR3*)&pos;
}


// �׸��ڸ� ����ϱ� ���� ������ �����Ѵ�.
// modelPos : �׸��ڸ� ����� ���� ��ġ (�������)
// lightPos : ������ ��ġ�� ����Ǿ� ����.
// view : �������� ���� �ٶ󺸴� �� ���
// proj : �������� ���� �ٶ󺸴� ���� ���
// tt : ���� ��ǥ���� �ؽ��� ��ǥ�� ��ȯ�ϴ� ���.
void cLight::GetShadowMatrix( const Vector3 &modelPos, 
	OUT Vector3 &lightPos, OUT Matrix44 &view, OUT Matrix44 &proj, 
	OUT Matrix44 &tt )
{
	if (D3DLIGHT_DIRECTIONAL == m_light.Type)
	{
		// ���⼺ �����̸� Direction ���͸� ���� ��ġ�� ����ϰ� �Ѵ�.
		Vector3 pos = *(Vector3*)&m_light.Position;
		Vector3 dir = *(Vector3*)&m_light.Direction;
		lightPos = -dir * pos.Length();
	}
	else
	{
		lightPos = *(Vector3*)&m_light.Position;
	}

	view.SetView2( lightPos, modelPos, Vector3(0,1,0));

	//proj.SetProjection( D3DX_PI/8.f, 1, 0.1f, 10000);
	//proj.SetProjectionOrthogonal(30, 30, 0.1f, 10000);
	proj.SetProjectionOrthogonal(50, 50, 0.1f, 10000.f);

	D3DXMATRIX mTT= D3DXMATRIX(0.5f, 0.0f, 0.0f, 0.0f
		, 0.0f,-0.5f, 0.0f, 0.0f
		, 0.0f, 0.0f, 1.0f, 0.0f
		, 0.5f, 0.5f, 0.0f, 1.0f);
	tt = *(Matrix44*)&mTT;
}


// �ӽ÷� ������ lightPos �� �������� �׸��� ����� �����.
// lightPos�� ������� �ʴ´�.
void cLight::GetShadowMatrix(const Vector3 &lightPos, OUT Matrix44 &view, OUT Matrix44 &proj, OUT Matrix44 &tt)
{
	Vector3 dir = *(Vector3*)&m_light.Direction;
	Vector3 modelPos = dir * 10 + lightPos;

	view.SetView2(lightPos, modelPos, Vector3(0, 1, 0));
	proj.SetProjectionOrthogonal(40, 40, 0.1f, 10000.f);

	D3DXMATRIX mTT = D3DXMATRIX(0.5f, 0.0f, 0.0f, 0.0f
		, 0.0f, -0.5f, 0.0f, 0.0f
		, 0.0f, 0.0f, 1.0f, 0.0f
		, 0.5f, 0.5f, 0.0f, 1.0f);
	tt = *(Matrix44*)&mTT;
}


void cLight::GetShadowMatrix(OUT Matrix44 &view, OUT Matrix44 &proj, OUT Matrix44 &tt)
{
	Vector3 lightPos = *(Vector3*)&m_light.Position;
	Vector3 dir = *(Vector3*)&m_light.Direction;
	Vector3 modelPos = dir * 10 + lightPos;

	view.SetView2(lightPos, modelPos, Vector3(0, 1, 0));
	//proj.SetProjectionOrthogonal(80, 80, 0.1f, 10000.f);
	proj.SetProjectionOrthogonal(800, 800, 0.1f, 10000.f);

	D3DXMATRIX mTT = D3DXMATRIX(0.5f, 0.0f, 0.0f, 0.0f
		, 0.0f, -0.5f, 0.0f, 0.0f
		, 0.0f, 0.0f, 1.0f, 0.0f
		, 0.5f, 0.5f, 0.0f, 1.0f);
	tt = *(Matrix44*)&mTT;
}


void cLight::Bind(cRenderer &renderer, 
	int lightIndex) // 0
{
	renderer.GetDevice()->SetLight(lightIndex, &m_light);
}


// ���̴� ������ �����ÿ� ���õ� ������ �ʱ�ȭ �Ѵ�.
void cLight::Bind(cShader &shader)  const
{
	shader.SetLightDir(*(Vector3*)&m_light.Direction);
	shader.SetLightPos(*(Vector3*)&m_light.Position);
	shader.SetLightAmbient(*(Vector4*)&m_light.Ambient);
	shader.SetLightDiffuse(*(Vector4*)&m_light.Diffuse);
	shader.SetLightSpecular(*(Vector4*)&m_light.Specular);
	shader.SetLightTheta(m_light.Theta);
	shader.SetLightPhi(m_light.Phi);
}
