//
// 2017-05-13, jjuiddong
// Debug Box 
//		- Vertex = Position + Diffuse 로 표현된다.
//		- Triangle Render
//
#pragma once


namespace graphic
{
	class cRenderer;

	class cDbgBox
	{
	public:
		cDbgBox();
		cDbgBox(cRenderer &renderer, const Vector3 &vMin, const Vector3 &vMax);

		void InitBox(cRenderer &renderer);
		void InitBox(cRenderer &renderer, Vector3 vertices[8]);
		void SetBox(cRenderer &renderer, const Vector3 &vMin, const Vector3 &vMax);
		void SetBox(Vector3 vertices[8]);
		void SetBox(const Vector3 &vMin, const Vector3 &vMax);
		void SetBox(const cBoundingBox &bbox);
		void SetColor(DWORD color);
		void Render(cRenderer &renderer, const Matrix44 &tm = Matrix44::Identity);


	public:
		cVertexBuffer m_vtxBuff;
		cIndexBuffer m_idxBuff;
		Vector3 m_min;
		Vector3 m_max;
		Matrix44 m_tm;
	};

}
