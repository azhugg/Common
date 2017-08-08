//
// 2017-05-06, jjuiddong
// �簢���� ��Ÿ���� Ŭ������.
// �����ϰ� �ؽ��ĸ� ���� �� ����� �� �ִ� Ŭ����.
// Vertex = Point + UV
//
// 2017-08-07
//	- Upgrade DX11
//
//
#pragma once


namespace graphic
{

	class cQuad : public cNode2
	{
	public:
		cQuad();
		cQuad(cRenderer &renderer, const float width, const float height, const Vector3 &pos
			, const int vtxType = (eVertexType::POSITION | eVertexType::TEXTURE)
			, const StrPath &textureFileName = "");
		virtual ~cQuad();

		bool Create(cRenderer &renderer, const float width, const float height, const Vector3 &pos
			, const int vtxType = (eVertexType::POSITION | eVertexType::TEXTURE)
			, const StrPath &textureFileName = "");

		virtual bool Render(cRenderer &renderer, const XMMATRIX &parentTm = XMIdentity, const int flags = 1) override;


	public:
		cQuadShape m_shape;
		cTexture *m_texture; // reference
	};

}
