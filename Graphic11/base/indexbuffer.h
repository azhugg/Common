//
// 2017-07-27, jjuiddong
// Upgrade Dx9 - Dx11
// IndexBuffer Class
//
#pragma once


namespace graphic
{
	class cRenderer;

	class cIndexBuffer
	{
	public:
		cIndexBuffer();
		cIndexBuffer(const cIndexBuffer &rhs);
		virtual ~cIndexBuffer();

		bool Create(cRenderer &renderer, const int faceCount
			, const D3D11_USAGE usage = D3D11_USAGE_DEFAULT
			, const DXGI_FORMAT fmt = DXGI_FORMAT_R16_UINT);

		bool Create(cRenderer &renderer, const int faceCount, BYTE *indices
			, const D3D11_USAGE usage = D3D11_USAGE_DEFAULT
			, const DXGI_FORMAT fmt = DXGI_FORMAT_R16_UINT);

		bool Create2(cRenderer &renderer, const int primitiveCount, const int primitiveSize);
		void* Lock();
		void Unlock();
		void Bind(cRenderer &renderer) const;
		int GetFaceCount() const;
		void Clear();

		void Set(cRenderer &renderer, const cIndexBuffer &rhs);
		cIndexBuffer& operator=(const cIndexBuffer &rhs);


	public:
		ID3D11Buffer *m_idxBuff;
		int m_faceCount;
		DXGI_FORMAT m_format;
	};


	inline int cIndexBuffer::GetFaceCount() const { return m_faceCount; }
}
