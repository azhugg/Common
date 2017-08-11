//
// 2017-05-15, jjuiddong
// ����ü ����� Ŭ����.
//
#pragma once


namespace graphic
{
	class cRenderer;

	class cDbgFrustum : public cFrustum
	{
	public:
		cDbgFrustum();
		virtual ~cDbgFrustum();

		bool Create(cRenderer &renderer, const Vector3 &pos, const Vector3 &direction
			, const Matrix44 &proj, const cColor &color = cColor::BLACK);
		bool Create(cRenderer &renderer, const cCamera &camera, const cColor &color = cColor::BLACK);
		bool SetFrustum(cRenderer &renderer, const Vector3 &pos, const Vector3 &direction
			, const Matrix44 &proj, const cColor &color = cColor::BLACK);
		bool SetFrustum(cRenderer &renderer, const cFrustum &frustum);
		bool SetFrustum(cRenderer &renderer, const cCamera &camera);
		void Render(cRenderer &renderer);


	protected:
		bool SetFrustum(cRenderer &renderer, const Matrix44 &viewProj, const cColor &color = cColor::BLACK);


	public:
		cDbgBox m_box;
	};

}
