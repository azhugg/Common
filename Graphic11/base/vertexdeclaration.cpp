
#include "stdafx.h"
#include "vertexdeclaration.h"


using namespace graphic;


cVertexDeclaration::cVertexDeclaration()
	: m_vertexLayout(NULL)
{
}

cVertexDeclaration::~cVertexDeclaration()
{
	SAFE_RELEASE(m_vertexLayout);
}


bool cVertexDeclaration::Create(cRenderer &renderer, ID3DBlob *vsBlob 
	, const D3D11_INPUT_ELEMENT_DESC layout[], const int numElements)
{
	m_decl.clear();

	int size = 0;
	if (FAILED(renderer.GetDevice()->CreateInputLayout(layout, numElements
		, vsBlob->GetBufferPointer()
		, vsBlob->GetBufferSize()
		, &m_vertexLayout)))
		return false;

	m_elementSize = size;
	return true;
}


bool cVertexDeclaration::Create(const sRawMesh &rawMesh )
{
	CreateDecl(rawMesh.vertices,
		rawMesh.normals,
		rawMesh.tex,
		rawMesh.tangent,
		rawMesh.binormal,
		rawMesh.weights);

	return true;
}


bool cVertexDeclaration::Create(const sRawMesh2 &rawMesh)
{
	CreateDecl(rawMesh.vertices,
		rawMesh.normals,
		rawMesh.tex,
		rawMesh.tangent,
		rawMesh.binormal,
		rawMesh.weights);

	return true;
}


void cVertexDeclaration::CreateDecl(
	const vector<Vector3> &vertices,
	const vector<Vector3> &normals,
	const vector<Vector3> &tex,
	const vector<Vector3> &tangent,
	const vector<Vector3> &binormal,
	const vector<sVertexWeight> &weights
	)
{
	int offset = 0;
	m_decl.clear();

	//if (!vertices.empty())
	//{
	//	D3DVERTEXELEMENT9 element = { 0,  (WORD)offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 };
	//	m_decl.push_back(element);
	//	offset += 12;
	//}

	//if (!normals.empty())
	//{
	//	D3DVERTEXELEMENT9 element = { 0, (WORD)offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 };
	//	m_decl.push_back(element);
	//	offset += 12;
	//}

	//if (!tex.empty())
	//{
	//	D3DVERTEXELEMENT9 element = { 0, (WORD)offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 };
	//	m_decl.push_back(element);
	//	offset += 8;
	//}

	//if (!tangent.empty())
	//{
	//	D3DVERTEXELEMENT9 element = { 0, (WORD)offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 };
	//	m_decl.push_back(element);
	//	offset += 12;
	//}

	//if (!binormal.empty())
	//{
	//	D3DVERTEXELEMENT9 element = { 0, (WORD)offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,  0 };
	//	m_decl.push_back(element);
	//	offset += 12;
	//}

	//if (!weights.empty())
	//{
	//	// �ؽ��� ��ǥ�� ���� ������ �����Ѵ�.
	//	// blend Weight 4
	//	D3DVERTEXELEMENT9 element1 = { 0, (WORD)offset, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 };
	//	m_decl.push_back(element1);
	//	offset += 16;

	//	// blend Indices 4
	//	D3DVERTEXELEMENT9 element2 = { 0, (WORD)offset, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  2 };
	//	m_decl.push_back(element2);
	//	offset += 16;
	//}

	//D3DVERTEXELEMENT9 endElement = D3DDECL_END();
	//m_decl.push_back(endElement);

	//m_elementSize = offset;
}


int cVertexDeclaration::GetOffset( const BYTE usage, const BYTE usageIndex ) const //usageIndex=0
{
	//for (u_int i=0; i < m_decl.size(); ++i)
	//{
	//	if ((usage == m_decl[ i].Usage) && (usageIndex == m_decl[ i].UsageIndex))
	//		return m_decl[ i].Offset;
	//}
	return -1;
}


void cVertexDeclaration::Bind(cRenderer &renderer)
{
	assert(m_vertexLayout);
	renderer.GetDeviceContext()->IASetInputLayout(m_vertexLayout);
}
