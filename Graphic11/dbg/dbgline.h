//
// 2017-05-13, jjuiddong
// ������ü �޽����ؼ� ������ �׸���.
// Vertex = Point + Diffuse
//
// 2017-08-07
//	- Upgrade DX11
//
#pragma once


namespace graphic
{

	class cDbgLine : public cLine
	{
	public:
		cDbgLine();
		cDbgLine(cRenderer &renderer
			, const Vector3 &p0 = Vector3(0, 0, 0)
			, const Vector3 &p1 = Vector3(1, 1, 1)
			, const float width = 1.f
			, const cColor color = cColor::BLACK);

		bool Create(cRenderer &renderer
			, const Vector3 &p0 = Vector3(0, 0, 0)
			, const Vector3 &p1 = Vector3(1, 1, 1)
			, const float width = 1.f
			, const cColor color = cColor::BLACK);

		virtual bool Render(cRenderer &renderer, const XMMATRIX &parentTm = XMIdentity, const int flags = 1);


	public:
		bool m_isSolid;
	};

}
