
#include "stdafx.h"
#include "frustum.h"

using namespace graphic;

cFrustum::cFrustum()
:	m_fullCheck(false)
,	m_plane(6) // ����ü ��� 6��
, m_epsilon(5.f)
{
}

cFrustum::~cFrustum()
{
}


//-----------------------------------------------------------------------------//
// ī�޶�(view) * ��������(projection)����� �Է¹޾� 6���� ����� �����.
//-----------------------------------------------------------------------------//
bool cFrustum::SetFrustum(const Matrix44 &matViewProj)
{
	//        4 --- 5
	//      / |  |  /|
	//   0 --- 1   |
	//   |   6-|- -7
	//   | /     | /
	//   2 --- 3
	//
	Vector3 vertices[8] = {
		Vector3(-1,1,0), Vector3(1,1,0), Vector3(-1,-1,0), Vector3(1,-1,0),
		Vector3(-1,1, 1), Vector3(1,1, 1), Vector3(-1,-1,1), Vector3(1,-1,1),
	};

	// view * proj�� ������� ���Ѵ�.
	Matrix44 matInv = matViewProj.Inverse();

	// Vertex_���� = Vertex_local * Matrix_world * Matrix_view * Matrix_Proj �ε�,
	// Vertex_world = Vertex_local * Matrix_world�̹Ƿ�,
	// Vertex_���� = Vertex_world * Matrix_view * Matrix_Proj �̴�.
	// Vertex_���� = Vertex_world * ( Matrix_view * Matrix_Proj ) ����
	// �����( Matrix_view * Matrix_Proj )^-1�� �纯�� ���ϸ�
	// Vertex_���� * �����( Matrix_view * Matrix_Proj )^-1 = Vertex_World �� �ȴ�.
	// �׷��Ƿ�, m_Vtx * matInv = Vertex_world�� �Ǿ�, ������ǥ���� �������� ��ǥ�� ���� �� �ִ�.
	//m_fullCheck = false;

	for (int i = 0; i < 8; i++)
		vertices[i] *= matInv;

	// 0���� 3���� ���������� near����� ������ܰ� �����ϴ��̹Ƿ�, ���� ��ǥ�� ���ؼ� 2�� ������
	// ī�޶��� ��ǥ�� ���� �� �ִ�.(��Ȯ�� ��ġ�ϴ� ���� �ƴϴ�.)
	m_pos = ( vertices[0] + vertices[3] ) / 2.0f;

	// ����� ������ǥ�� �������� ����� �����
	// ���Ͱ� �������� ���ʿ��� �ٱ������� ������ �����̴�.
	m_plane[0].Init(vertices[0], vertices[1], vertices[2]);	// �� ���(near)
	m_plane[1].Init(vertices[0], vertices[4], vertices[1]);	// �� ���(up)
	m_plane[2].Init(vertices[2], vertices[3], vertices[7]);	// �Ʒ� ���(down)
	m_plane[3].Init( vertices[ 4], vertices[ 6], vertices[ 5] );	// �� ���(far)
	m_plane[4].Init( vertices[ 0], vertices[ 2], vertices[ 6] );	// �� ���(left)
	m_plane[5].Init( vertices[ 1], vertices[ 5], vertices[ 7] );	// �� ���(right)

	m_viewProj = matViewProj;

	return TRUE;
}


//-----------------------------------------------------------------------------//
// ������ü�� minimum pos �� maximum pos �� ����ü�� �����.
//-----------------------------------------------------------------------------//
bool cFrustum::SetFrustum(const Vector3 &_min, const Vector3 &_max)
{
	//        4 --- 5
	//      / |  |  /|
	//   0 --- 1   |
	//   |   6-|- -7
	//   | /     | /
	//   2 --- 3
	//
	Vector3 vertices[8] = {
		Vector3(_min.x, _max.y, _min.z), Vector3(_max.x,_max.y,_min.z), Vector3(_min.x,_min.y,_min.z), Vector3(_max.x,_min.y,_min.z),
		Vector3(_min.x, _max.y, _max.z), Vector3(_max.x,_max.y,_max.z), Vector3(_min.x,_min.y,_max.z), Vector3(_max.x,_min.y,_max.z),
	};

	m_fullCheck = true;

	m_pos = (_min + _max) / 2.0f;

	// ����� ������ǥ�� �������� ����� �����
	// ���Ͱ� �������� ���ʿ��� �ٱ������� ������ �����̴�.
	m_plane[0].Init(vertices[0], vertices[1], vertices[2]);	// �� ���(near)
	m_plane[1].Init(vertices[0], vertices[4], vertices[1]);	// �� ���(up)
	m_plane[2].Init(vertices[2], vertices[3], vertices[7]);	// �Ʒ� ���(down)
	m_plane[3].Init(vertices[4], vertices[6], vertices[5]);	// �� ���(far)
	m_plane[4].Init(vertices[0], vertices[2], vertices[6]);	// �� ���(left)
	m_plane[5].Init(vertices[1], vertices[5], vertices[7]);	// �� ���(right)

	return true;
}


//-----------------------------------------------------------------------------//
// ���� point�� �������Ҿȿ� ������ TRUE�� ��ȯ, �ƴϸ� FALSE�� ��ȯ�Ѵ�.
//-----------------------------------------------------------------------------//
bool cFrustum::IsIn( const Vector3 &point ) const
{
	for (int i=0; i < 6; ++i)
	{
		// m_fullCheck �� false ��� near, top, bottom  ��� üũ�� ���� �ȴ�.
		if (!m_fullCheck && (i < 3))
			continue;

		const float dist = m_plane[ i].Distance( point );
		if (dist > m_epsilon)
			return false;
	}

	return true;
}


//-----------------------------------------------------------------------------//
// �߽�(point)�� ������(radius)�� ���� ��豸(bounding sphere)�� �������Ҿȿ� ������
// TRUE�� ��ȯ, �ƴϸ� FALSE�� ��ȯ�Ѵ�.
//-----------------------------------------------------------------------------//
bool cFrustum::IsInSphere( const Vector3 &point, float radius
	, const Matrix44 &tm //= Matrix44::Identity
) const
{
	const Vector3 pos = point * tm;

	for (int i=0; i < 6; ++i)
	{
		// m_fullCheck �� false ��� near, top, bottom  ��� üũ�� ���� �ȴ�.
		if (!m_fullCheck && (i < 3))
			continue;

		// ���� �߽����� �Ÿ��� ���������� ũ�� �������ҿ� ����
		const float dist = m_plane[ i].Distance( pos );
		if (dist > (radius+ m_epsilon))
			return false;
	}

	return true;
}


bool cFrustum::IsInSphere(const cBoundingSphere &sphere
	, const Matrix44 &tm //= Matrix44::Identity
) const
{
	return IsInSphere(sphere.m_pos, sphere.m_radius, tm);
}


bool cFrustum::IsInBox(const cBoundingBox &bbox) const
{
	//const Vector3 vertices[] = {
	//	bbox.m_min
	//	, Vector3(bbox.m_max.x, bbox.m_min.y, bbox.m_min.z)
	//	, Vector3(bbox.m_min.x, bbox.m_max.y, bbox.m_min.z)
	//	, Vector3(bbox.m_min.x, bbox.m_max.y, bbox.m_max.z)
	//	, bbox.m_max
	//	, Vector3(bbox.m_min.x, bbox.m_max.y, bbox.m_max.z)
	//	, Vector3(bbox.m_max.x, bbox.m_min.y, bbox.m_max.z)
	//	, Vector3(bbox.m_max.x, bbox.m_max.y, bbox.m_min.z)
	//};

	//for (int i = 0; i < 8; ++i)
	//{
	//	if (IsIn(vertices[i]))
	//		return true;
	//}

	Vector3 size = bbox.m_max - bbox.m_min;
	const float radius = size.Length() * 0.5f;
	return IsInSphere(bbox.Center(), radius);
}


// Split Frustum
// f : 0 ~ 1
void cFrustum::Split2(cCamera &cam, const float f1, const float f2
	, cFrustum &out1, cFrustum &out2)
{
	const float oldNearPlane = cam.m_nearPlane;
	const float oldFarPlane = cam.m_farPlane;
	const float far1 = common::lerp(cam.m_nearPlane, cam.m_farPlane, f1);
	const float far2 = common::lerp(cam.m_nearPlane, cam.m_farPlane, f2); 

	cam.ReCalcProjection(cam.m_nearPlane, far1);
	out1.SetFrustum(cam.GetViewProjectionMatrix());
	
	cam.ReCalcProjection(far1, far2);
	out2.SetFrustum(cam.GetViewProjectionMatrix());

	// recovery
	cam.ReCalcProjection(oldNearPlane, oldFarPlane);
}


// Split Frustum
// f : 0 ~ 1
void cFrustum::Split3(cCamera &cam, const float f1, const float f2, const float f3
	, cFrustum &out1, cFrustum &out2, cFrustum &out3)
{
	const float oldNearPlane = cam.m_nearPlane;
	const float oldFarPlane = cam.m_farPlane;
	const float far1 = common::lerp(cam.m_nearPlane, cam.m_farPlane, f1);
	const float far2 = common::lerp(cam.m_nearPlane, cam.m_farPlane, f2);
	const float far3 = common::lerp(cam.m_nearPlane, cam.m_farPlane, f3);

	cam.ReCalcProjection(cam.m_nearPlane, far1);
	out1.SetFrustum(cam.GetViewProjectionMatrix());

	cam.ReCalcProjection(far1, far2);
	out2.SetFrustum(cam.GetViewProjectionMatrix());

	cam.ReCalcProjection(far2, far3);
	out3.SetFrustum(cam.GetViewProjectionMatrix());

	// recovery
	cam.ReCalcProjection(oldNearPlane, oldFarPlane);
}


// return Frustum & Plane Collision Vertex
// Plane is Usually Ground Plane
void cFrustum::GetGroundPlaneVertices(const Plane &plane, OUT Vector3 outVertices[4]) const
{
	//        4 --- 5
	//      / |  |  /|
	//   0 --- 1   |
	//   |   6-|- -7
	//   | /     | /
	//   2 --- 3
	//
	Vector3 vertices[8] = {
		Vector3(-1,1,0), Vector3(1,1,0), Vector3(-1,-1,0), Vector3(1,-1,0),
		Vector3(-1,1, 1), Vector3(1,1, 1), Vector3(-1,-1,1), Vector3(1,-1,1),
	};
	Matrix44 matInv = m_viewProj.Inverse();
	for (int i = 0; i < 8; ++i)
		vertices[i] *= matInv;

	// Far Plane
	Vector3 p0, p1;
	if (plane.LineCross(vertices[0], vertices[4], &p0) == 0)
		p0 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	if (plane.LineCross(vertices[1], vertices[5], &p1) == 0)
		p1 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 p4 = plane.Pick(vertices[4], (vertices[6] - vertices[4]).Normal());
	Vector3 p5 = plane.Pick(vertices[5], (vertices[7] - vertices[5]).Normal());

	// Near Plane
	Vector3 p2, p3;
	if (plane.LineCross(vertices[3], vertices[7], &p2) == 0)
		p2 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	if (plane.LineCross(vertices[2], vertices[6], &p3) == 0)
		p3 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 p6 = plane.Pick(vertices[1], (vertices[3] - vertices[1]).Normal());
	Vector3 p7 = plane.Pick(vertices[0], (vertices[2] - vertices[0]).Normal());

	outVertices[0] = (m_pos.LengthRoughly(p0) < m_pos.LengthRoughly(p4)) ? p0 : p4;
	outVertices[1] = (m_pos.LengthRoughly(p1) < m_pos.LengthRoughly(p5)) ? p1 : p5;
	outVertices[2] = (m_pos.LengthRoughly(p2) < m_pos.LengthRoughly(p6)) ? p2 : p6;
	outVertices[3] = (m_pos.LengthRoughly(p3) < m_pos.LengthRoughly(p7)) ? p3 : p7;
}
