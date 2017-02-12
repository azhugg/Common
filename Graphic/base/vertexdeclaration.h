// ���ؽ� ����ü ������ ǥ���ϴ� Ŭ����
// D3DVERTEXELEMENT9 ����Ÿ�� ���� ����ϱ����� �������.
#pragma once


namespace graphic
{

	class cVertexDeclaration
	{
	public:
		cVertexDeclaration();
		virtual ~cVertexDeclaration();

		bool Create(const sRawMesh &rawMesh );
		bool Create(const sRawMesh2 &rawMesh);
		int GetOffset( const BYTE usage, const BYTE usageIndex=0 ) const;
		const vector<D3DVERTEXELEMENT9>& GetDecl() const;
		int GetElementSize() const;


	protected:
		void CreateDecl(
			const vector<Vector3> &vertices,
			const vector<Vector3> &normals,
			const vector<Vector3> &tex,
			const vector<Vector3> &tangent,
			const vector<Vector3> &binormal,
			const vector<sVertexWeight> &weights
			);


	public:
		vector<D3DVERTEXELEMENT9> m_decl;
		int m_elementSize;
	};


	inline const vector<D3DVERTEXELEMENT9>& cVertexDeclaration::GetDecl() const { return m_decl; }
	inline int cVertexDeclaration::GetElementSize() const { return m_elementSize; }
}
