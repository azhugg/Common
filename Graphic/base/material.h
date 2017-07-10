// ���� Ŭ����
// D3DMATERIAL9 ����Ÿ�� ���� �ٷ������ �������.
#pragma once


namespace graphic
{
	struct sMaterial;
	class cShader;
	class cRenderer;

	class cMaterial
	{
	public:
		cMaterial();
		virtual ~cMaterial();
		
		void Init(const Vector4 &ambient, 
			const Vector4 &diffuse,
			const Vector4 &specular,
			const Vector4 &emmisive=Vector4(0,0,0,1),
			const float pow=0);

		void Init(const D3DMATERIAL9 &mtrl);
		void Init(const sMaterial &mtrl);

		D3DMATERIAL9& GetMtrl();

		void InitWhite();
		void InitBlack();
		void InitGray();
		void InitGray2();
		void InitGray3();
		void InitXFile();
		void InitRed();
		void InitBlue();
		void InitGreen();
		void InitYellow();
		void Bind(cRenderer &renderer);
		void Bind(cShader &shader);

		Str64 DiffuseColor();
		Str64 AmbientColor();
		Str64 SpecularColor();
		Str64 EmissiveColor();
		Str64 SpecialColor(const float r, const float g, const float b);


	public:
		D3DMATERIAL9 m_mtrl;
	};


	inline D3DMATERIAL9& cMaterial::GetMtrl() { return m_mtrl; }
}
