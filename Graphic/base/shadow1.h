//
// 2017-05-07, jjuiddong refactoring
// ���� �׸��� Ŭ����
// �� �ϳ��� �׸��� �ؽ��ĸ� �����.
// �ϳ� �̻��� ���� �׸��ڸ� ǥ�� �� ���� �� Ŭ������ ������� �ʴ°� ����.
// ���� ���� �������� �ʴ�, true/false �׸��� �ؽ��ĸ� �̿��Ѵ�.
//
#pragma once


namespace graphic
{
	class cModel2;

	class cShadow1
	{
	public:
		cShadow1();
		virtual ~cShadow1();

		bool Create(cRenderer &renderer, const int textureWidth, const int textureHeight);
		void UpdateShadow(cRenderer &renderer, cNode &node);
		void UpdateShadow(cRenderer &renderer, cModel2 &model);
		void Bind(cShader &shader, const string &key);
		void Begin(cRenderer &renderer);
		void End();
		IDirect3DTexture9* GetTexture();
		const IDirect3DTexture9* GetTexture() const;
		bool IsLoaded() const;
		void LostDevice();
		void ResetDevice(graphic::cRenderer &renderer);

		// ������ �Լ�.
		void RenderShadowMap(cRenderer &renderer);


	public:
		cSurface2 m_surface;
	};


	inline IDirect3DTexture9* cShadow1::GetTexture() { return m_surface.GetTexture(); }
	inline const IDirect3DTexture9* cShadow1::GetTexture() const { return m_surface.GetTexture(); }
	inline bool cShadow1::IsLoaded() const { return m_surface.IsLoaded(); }
}
