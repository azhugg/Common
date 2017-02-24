
#include "stdafx.h"
#include "camera.h"


using namespace graphic;


cCamera::cCamera() :
	m_eyePos(0,100,-100)
,	m_lookAt(0,0,0)
,	m_up(0,1,0)
, m_renderer(NULL)
, m_width(0)
, m_height(0)
{
	UpdateViewMatrix();

}

cCamera::cCamera(const Vector3 &eyePos, const Vector3 &lookAt, const Vector3 &up) :
	m_eyePos(eyePos)
,	m_lookAt(lookAt)
,	m_up(up)
, m_renderer(NULL)
{
	UpdateViewMatrix();
}

cCamera::~cCamera()
{
}


// 카메라 파라메터를 초기화하고, 카메라 행렬을 만든다.
void cCamera::SetCamera(const Vector3 &eyePos, const Vector3 &lookAt, const Vector3 &up)
{
	m_eyePos = eyePos;
	m_lookAt = lookAt;
	m_up = up;
	UpdateViewMatrix();
}


void cCamera::SetProjection(const float fov, const float aspect, const float nearPlane, const float farPlane)
{
	m_fov = fov;
	m_aspect = aspect;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	m_proj.SetProjection(fov, aspect, nearPlane, farPlane);
	UpdateProjectionMatrix();
}


// Direction 단위 벡터를 리턴한다.
Vector3 cCamera::GetDirection() const 
{
	Vector3 v = m_lookAt - m_eyePos;
	return v.Normal();
}


// Right 단위 벡터를 리턴한다.
Vector3 cCamera::GetRight() const
{
	Vector3 v = m_up.CrossProduct( GetDirection() );
	return v.Normal();
}


void cCamera::Update()
{
	
}


// Rendering Camera Position, Direction
void cCamera::Render(cRenderer &renderer)
{
	m_lines[0].SetLine(renderer, m_eyePos, m_eyePos + GetRight(), 0.1f, D3DCOLOR_XRGB(255,0,0) );
	m_lines[1].SetLine(renderer, m_eyePos, m_eyePos + GetUpVector(), 0.1f, D3DCOLOR_XRGB(0, 255, 0));
	m_lines[2].SetLine(renderer, m_eyePos, m_eyePos + GetDirection(), 0.1f, D3DCOLOR_XRGB(0, 0, 255));

	for (auto &line : m_lines)
		line.Render(renderer);
}


// shader bind
void cCamera::Bind(cShader &shader)
{
	shader.SetMatrix("g_mView", m_view);
	shader.SetMatrix("g_mProj", m_proj);
	shader.SetMatrix("g_mVP", m_view * m_proj);
	shader.SetVector("g_vEyePos", m_eyePos);
}


void cCamera::UpdateViewMatrix()
{
	RET(!m_renderer);

	m_view.SetView2(m_eyePos, m_lookAt, m_up);

	// Update Up Vector
	Vector3 dir = m_lookAt - m_eyePos;
	dir.Normalize();
	Vector3 right = m_up.CrossProduct(dir);
	right.Normalize();
	m_up = dir.CrossProduct(right);
	m_up.Normalize();
	//

	if (m_renderer->GetDevice())
		m_renderer->GetDevice()->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_view);
}


// ViewMatrix 로 eyePos, lookAt, up 을 계산한다.
void cCamera::UpdateParameterFromViewMatrix()
{
	const Vector3 right(m_view._11, m_view._21, m_view._31);
	const Vector3 up(m_view._12, m_view._22, m_view._32 );
	const Vector3 dir(m_view._13, m_view._23, m_view._33);
	const Vector3 pos = m_view.Inverse().GetPosition();
	
	m_eyePos = pos;
	m_up = up;
	m_lookAt = dir * 100 + pos;
}


void cCamera::UpdateProjectionMatrix()
{
	RET(!m_renderer);

	if (m_renderer->GetDevice())
		m_renderer->GetDevice()->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m_proj);
}


// Right 축으로 회전한다.
void cCamera::Pitch( const float radian )
{
	RET(radian == 0);

	const Vector3 axis = GetRight();
	const Quaternion q( axis, radian );
	const Matrix44 mat = q.GetMatrix();

	Vector3 v = m_lookAt - m_eyePos;
	v *= mat;
	m_lookAt = m_eyePos + v;

	UpdateViewMatrix();
}


// Up 축으로 회전한다.
void cCamera::Yaw( const float radian )
{
	RET(radian == 0);

	const Vector3 axis = GetUpVector();
	const Quaternion q( axis, radian );
	const Matrix44 mat = q.GetMatrix();

	Vector3 v = m_lookAt - m_eyePos;
	v *= mat;
	m_lookAt = m_eyePos + v;

	UpdateViewMatrix();	
}


// Direction 축으로 회전한다.
void cCamera::Roll( const float radian )
{
	RET(radian == 0);

	const Vector3 axis = GetDirection();
	const Quaternion q( axis, radian );
	const Matrix44 mat = q.GetMatrix();

	Vector3 v = m_lookAt - m_eyePos;
	v *= mat;
	m_lookAt = m_eyePos + v;

	UpdateViewMatrix();	
}



// Right 축으로 회전한다.
void cCamera::Pitch2( const float radian )
{
	RET(radian == 0);

	const Vector3 axis = GetRight();
	const Quaternion q( axis, radian );
	const Matrix44 mat = q.GetMatrix();

	Vector3 v = m_eyePos - m_lookAt;
	v *= mat;
	m_eyePos = m_lookAt + v;

	UpdateViewMatrix();
}


// Up 축으로 회전한다.
void cCamera::Yaw2( const float radian )
{
	RET(radian == 0);

	const Vector3 axis = GetUpVector();
	const Quaternion q( axis, radian );
	const Matrix44 mat = q.GetMatrix();

	Vector3 v = m_eyePos - m_lookAt;
	v *= mat;
	m_eyePos = m_lookAt + v;

	UpdateViewMatrix();
}


// Direction 축으로 회전한다.
void cCamera::Roll2( const float radian )
{
	RET(radian == 0);

	const Vector3 axis = GetDirection();
	const Quaternion q( axis, radian );
	const Matrix44 mat = q.GetMatrix();

	Vector3 v = m_lookAt - m_eyePos;
	v *= mat;
	m_lookAt = m_eyePos + v;

	UpdateViewMatrix();	
}


// 앞/뒤으로 이동
void cCamera::MoveFront( const float len )
{
	const Vector3 dir = GetDirection();
	m_lookAt += dir * len;
	m_eyePos += dir * len;
	UpdateViewMatrix();
}


// 위아래 이동
void cCamera::MoveUp( const float len )
{
	const Vector3 dir = GetUpVector();
	m_lookAt += dir * len;
	m_eyePos += dir * len;
	UpdateViewMatrix();
}


// 좌우 이동.
void cCamera::MoveRight( const float len )
{
	const Vector3 dir = GetRight();
	m_lookAt += dir * len;
	m_eyePos += dir * len;
	UpdateViewMatrix();
}


// dir 방향으로 이동한다.
void cCamera::MoveAxis( const Vector3 &dir, const float len )
{
	m_lookAt += dir * len;
	m_eyePos += dir * len;
	UpdateViewMatrix();
}


// lookAt 은 고정된채로 eyePos 를 이동한다.
void cCamera::Zoom( const float len )
{
	const Vector3 dir = GetDirection();
	m_eyePos += dir * len;
	UpdateViewMatrix();
}


// lookAt - eyePos 사이 거리
float cCamera::GetDistance() const 
{
	return (m_lookAt - m_eyePos).Length();
}


Vector3 cCamera::GetScreenPos(const int viewportWidth, const int viewportHeight, const Vector3& vPos)
{
// 	Vector3 point = vPos * m_view * m_proj;
// 	point.x = (point.x + 1) * viewportWidth / 2;
// 	point.y = (-point.y + 1) * viewportHeight / 2;

	D3DVIEWPORT9 viewPort;
	viewPort.X = 0;
	viewPort.Y = 0;
	viewPort.Width = viewportWidth;
	viewPort.Height = viewportHeight;
	viewPort.MinZ = 0;
	viewPort.MaxZ = 1;

	Matrix44 world;

	Vector3 point;
	D3DXVec3Project((D3DXVECTOR3*)&point, (const D3DXVECTOR3*)&vPos, 
		&viewPort, (const D3DXMATRIX*)&m_proj, (const D3DXMATRIX*)&m_view, 
		(const D3DXMATRIX*)&world);

	return point;
}


void cCamera::GetRay(const int windowWidth, const int windowHeight, 
	const int sx, const int sy, Vector3 &orig, Vector3 &dir)
{
	const float x = ((sx * 2.0f / windowWidth) - 1.0f);
	const float y = -((sy * 2.0f / windowHeight) - 1.0f);

	Vector3 v;
	v.x = x / m_proj._11;
	v.y = y / m_proj._22;
	v.z = 1.0f;

	Matrix44 m = m_view.Inverse();

	dir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
	dir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
	dir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
	dir.Normalize();

	orig.x = m._41;
	orig.y = m._42;
	orig.z = m._43;
}


// return camera position, shadow matrix
void cCamera::GetShadowMatrix(OUT Matrix44 &view, OUT Matrix44 &proj, OUT Matrix44 &tt)
{
	view = GetViewMatrix();
	proj = GetProjectionMatrix();

	D3DXMATRIX mTT = D3DXMATRIX(0.5f, 0.0f, 0.0f, 0.0f
		, 0.0f, -0.5f, 0.0f, 0.0f
		, 0.0f, 0.0f, 1.0f, 0.0f
		, 0.5f, 0.5f, 0.0f, 1.0f);
	tt = *(Matrix44*)&mTT;
}


void cCamera::SetViewPort(const int width, const int height)
{
	m_width = width;
	m_height = height;
}
