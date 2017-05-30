
#include "stdafx.h"
#include "frustum.h"

using namespace graphic;

//#define PLANE_EPSILON		5.0f


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
bool cFrustum::IsInSphere( const Vector3 &point, float radius ) const
{
	for (int i=0; i < 6; ++i)
	{
		// m_fullCheck �� false ��� near, top, bottom  ��� üũ�� ���� �ȴ�.
		if (!m_fullCheck && (i < 3))
			continue;

		// ���� �߽����� �Ÿ��� ���������� ũ�� �������ҿ� ����
		const float dist = m_plane[ i].Distance( point );
		if (dist > (radius+ m_epsilon))
			return false;
	}

	return true;
}


bool cFrustum::IsInSphere(const cBoundingSphere &sphere) const
{
	return IsInSphere(sphere.m_pos, sphere.m_radius);
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
