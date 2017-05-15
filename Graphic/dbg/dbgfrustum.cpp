
#include "stdafx.h"
#include "dbgfrustum.h"

using namespace graphic;


cDbgFrustum::cDbgFrustum()
	: m_fullCheck(false)
{
}

cDbgFrustum::~cDbgFrustum()
{
}


//-----------------------------------------------------------------------------//
// ī�޶�(view) * ��������(projection)����� �Է¹޾� 6���� ����� �����.
//-----------------------------------------------------------------------------//
bool cDbgFrustum::Create(cRenderer &renderer, const Matrix44 &matViewProj)
{
	cFrustum::SetFrustum(matViewProj);
	SetFrustum(renderer, matViewProj);
	return TRUE;
}


//-----------------------------------------------------------------------------//
// ������ü�� minimum pos �� maximum pos �� ����ü�� �����.
//-----------------------------------------------------------------------------//
bool cDbgFrustum::Create(cRenderer &renderer, const Vector3 &_min, const Vector3 &_max)
{
	cFrustum::SetFrustum(_min, _max);

	m_box.SetBox(renderer, _min, _max);

	return true;
}


void cDbgFrustum::SetFrustum(cRenderer &renderer, const Matrix44 &matViewProj)
{
	cFrustum::SetFrustum(matViewProj);

	// ������ı��� ��ġ�� ��� 3���� ������ǥ�� ���� (-1,-1,0) ~ (1,1,1)������ ������ �ٲ��.
	Vector3 vertices[8] = {
		Vector3(-1,1,0), Vector3(1,1,0), Vector3(-1,-1,0), Vector3(1,-1,0),
		Vector3(-1,1, 1), Vector3(1,1, 1), Vector3(-1,-1,1), Vector3(1,-1,1),
	};

	// view * proj�� ������� ���Ѵ�.
	Matrix44 matInv = matViewProj.Inverse();

	for (int i = 0; i < 8; i++)
		vertices[i] *= matInv;

	m_box.InitBox(renderer, vertices);	
}


//-----------------------------------------------------------------------------//
// ���
//-----------------------------------------------------------------------------//
void cDbgFrustum::Render(cRenderer &renderer)
{
	m_box.Render(renderer);

	//WORD	index[] = { 0, 1, 2,
	//	0, 2, 3,
	//	4, 7, 6,
	//	4, 6, 5,
	//	1, 5, 6,
	//	1, 6, 2,
	//	0, 3, 7,
	//	0, 7, 4,
	//	0, 4, 5,
	//	0, 5, 1,
	//	3, 7, 6,
	//	3, 6, 2 };

	//D3DMATERIAL9 mtrl;
	//ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	//typedef struct tagVTX
	//{
	//	Vector3 p;
	//} VTX;

	//VTX vtx[8];

	//for( int i = 0 ; i < 8 ; i++ )
	//	vtx[i].p = m_vtx[i];

	//GetDevice()->SetFVF( D3DFVF_XYZ );
	//GetDevice()->SetStreamSource( 0, NULL, 0, sizeof(VTX) );
	//GetDevice()->SetTexture( 0, NULL );
	//GetDevice()->SetIndices( 0 );
	//GetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	//GetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	//GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	//GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	//GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	//// �Ķ������� ��,�� ����� �׸���.
	//GetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	//ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	//mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	//GetDevice()->SetMaterial( &mtrl );
	//GetDevice()->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 8, 4, index, D3DFMT_INDEX16, vtx, sizeof( vtx[0] ) );

	//// ������� ��,�� ����� �׸���.
	//ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	//mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	//GetDevice()->SetMaterial( &mtrl );
	//GetDevice()->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 8, 4, index+4*3, D3DFMT_INDEX16, vtx, sizeof( vtx[0] ) );

	//// ���������� ��,�� ����� �׸���.
	//ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	//mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	//GetDevice()->SetMaterial( &mtrl );
	//GetDevice()->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 8, 4, index+8*3, D3DFMT_INDEX16, vtx, sizeof( vtx[0] ) );

	//GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	//GetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
}
