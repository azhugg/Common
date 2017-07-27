// cMeshBuffer
// �޽��� �����ϴ� ���ؽ� ����, �ε��� ����, �Ӽ� ���۸� 
// �����ϴ� Ŭ������.
#pragma once


namespace graphic
{

	class cMeshBuffer
	{
	public:
		cMeshBuffer();
		cMeshBuffer(cRenderer &renderer, const sRawMesh &rawMesh);
		cMeshBuffer(cRenderer &renderer, const sRawMesh2 &rawMesh);
		cMeshBuffer(cRenderer &renderer, const sRawBone &rawBone);
		virtual ~cMeshBuffer();

		void Bind(cRenderer &renderer);
		void Render(cRenderer &renderer, const int faceStart = 0, const int faceCount = 0);

		cMeshBuffer* Clone();
		vector<sAttribute>& GetAttributes();
		cVertexBuffer& GetVertexBuffer();
		cIndexBuffer& GetIndexBuffer();
		const cBoundingBox& GetBoundingBox() const;
		void SetOffset(const int offset);
		int GetOffset() const;
		bool IsLoaded();
		void Clear();


	protected:
		void CreateMesh(cRenderer &renderer, const sRawMesh &rawMesh);
		void CreateMesh(cRenderer &renderer, const sRawMesh2 &rawMesh);
		
		void CreateMesh(cRenderer &renderer, 
			const vector<Vector3> &vertices,
			const vector<Vector3> &normals, 
			const vector<Vector3> &tex,
			const vector<int> &indices);

		void CreateMesh(cRenderer &renderer,
			const cVertexDeclaration &decl,
			const vector<Vector3> &vertices,
			const vector<Vector3> &normals,
			const vector<Vector3> &tex,
			const vector<Vector3> &tangent,
			const vector<Vector3> &binormal,
			const vector<sVertexWeight> &weights,
			const vector<int> &indices);

		void CreateAttributes(const sRawMesh &rawMesh);


	public:
		vector<sAttribute> m_attributes;
		cVertexBuffer m_vtxBuff;
		cIndexBuffer m_idxBuff;
		int m_offset;
		bool m_isSkinned; // skinning vertex
		cBoundingBox m_boundingBox;
	};


	inline vector<sAttribute>& cMeshBuffer::GetAttributes() { return m_attributes; }
	inline cVertexBuffer& cMeshBuffer::GetVertexBuffer() { return m_vtxBuff; }
	inline cIndexBuffer& cMeshBuffer::GetIndexBuffer() { return m_idxBuff; }
	inline const cBoundingBox& cMeshBuffer::GetBoundingBox() const { return m_boundingBox; }
	inline void cMeshBuffer::SetOffset(const int offset) { m_offset = offset; }
	inline int cMeshBuffer::GetOffset() const { return m_offset; }
}
