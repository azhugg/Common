//
// 2017-07-20, jjuiddong
// Render Axis
//
// 2017-08-08
//	- Upgrade DX11
//
#pragma once


namespace graphic
{

	class cDbgAxis
	{
	public:
		cDbgAxis();
		virtual ~cDbgAxis();

		bool Create(cRenderer &renderer);
		void SetAxis(const float size, const cBoundingBox &bbox, const bool approximate=true);
		void SetAxis(const cBoundingBox &bbox, const bool approximate = true);
		void Render(cRenderer &renderer, const XMMATRIX &tm=XMIdentity);


	public:
		cDbgLine m_lines[3]; // x-y-z
	};

}
