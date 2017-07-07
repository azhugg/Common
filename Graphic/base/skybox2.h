// ��ī�̹ڽ�
// Sphere �� �ϴ��� ǥ���ϰ�, ���̴��� ����Ѵ�.
// Frank Luna �� Sky Ŭ������ �������� �������.
// PondWater ���� Clipping Plane �� �������ؼ� ���̴��ε� ��ī�̹ڽ��� �ʿ��ߴ�.
#pragma once


namespace graphic
{

	class cSkyBox2
	{
	public:
		cSkyBox2();
		virtual ~cSkyBox2();

		bool Create(cRenderer &renderer, const StrPath &skyboxFileName, const float radius);
		void Render(cRenderer &renderer, const Matrix44 &tm = Matrix44::Identity);

	
	private:
		ID3DXMesh* m_sphere;
		float m_radius;
		cCubeTexture *m_envMap;
		cShader *m_shader; // reference
	};

}

