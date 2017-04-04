
#include "stdafx.h"
#include "texture.h"


using namespace graphic;


cTexture::cTexture() :
	m_texture(NULL)
	, m_isReferenceMode(false)
{
}

cTexture::~cTexture()
{
	Clear();
}


bool cTexture::Create(cRenderer &renderer, const string &fileName, bool isSizePow2)//isSizePow2=true
{
	Clear();

	m_fileName = fileName;

	if (isSizePow2)
	{
		if (FAILED(D3DXCreateTextureFromFileA(renderer.GetDevice(), fileName.c_str(), &m_texture)))
			return false;

		// �ؽ��� ������ ����.
		D3DSURFACE_DESC desc;
		if (SUCCEEDED(m_texture->GetLevelDesc(0, &desc)))
		{
			m_imageInfo.Width = desc.Width;
			m_imageInfo.Height = desc.Height;
			m_imageInfo.Format = desc.Format;
		}
	}
	else
	{
		return CreateEx(renderer, fileName);
	}

	return true;
}


bool cTexture::Create(cRenderer &renderer, const int width, const int height, const D3DFORMAT format)
{
	Clear();

	if (FAILED(renderer.GetDevice()->CreateTexture( width, height, 1, 0, format, 
		D3DPOOL_MANAGED, &m_texture, NULL )))
		return false;

	D3DLOCKED_RECT lockrect;
	m_texture->LockRect( 0, &lockrect, NULL, 0 );
	memset( lockrect.pBits, 0x00, lockrect.Pitch*height );
	m_texture->UnlockRect( 0 );

	m_imageInfo.Width = width;
	m_imageInfo.Height = height;
	m_imageInfo.Format = format;
	return true;
}


// D3DX_DEFAULT_NONPOW2 �ɼ��� �� ���¿��� �ؽ��ĸ� �����Ѵ�.
bool cTexture::CreateEx(cRenderer &renderer, const string &fileName)
{
	Clear();

	if (FAILED(D3DXCreateTextureFromFileExA(
		renderer.GetDevice(), fileName.c_str(),
		D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, // option On
		NULL, NULL, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0,
		&m_imageInfo,
		NULL,
		&m_texture)))
	{
		return false;
	}

	m_fileName = fileName;
	
	return true;
}


void cTexture::Bind(cRenderer &renderer, const int stage)
{
	renderer.GetDevice()->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	renderer.GetDevice()->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	renderer.GetDevice()->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	renderer.GetDevice()->SetTexture(stage, m_texture);
}


void cTexture::Bind(cShader &shader, const string &key)
{
	shader.SetTexture(key, *this);
}

void cTexture::Unbind(cRenderer &renderer, const int stage)
{
	renderer.GetDevice()->SetTexture(stage, NULL);
}


void cTexture::Render2D(cRenderer &renderer)
{
	renderer.GetDevice()->SetTransform(D3DTS_WORLD, ToDxM(Matrix44::Identity));
	renderer.GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	renderer.GetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	renderer.GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	renderer.GetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	renderer.GetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	renderer.GetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	renderer.GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	typedef struct { FLOAT p[4]; FLOAT tu, tv; } TVERTEX;
	TVERTEX Vertex[4] = {
		// x  y  z rhw tu tv
		{ 0, 0, 0, 1, 0, 0, },
		{ (float)m_imageInfo.Width, 0,0, 1, 1, 0, },
		{ (float)m_imageInfo.Width, (float)m_imageInfo.Height, 1, 1, 1, 1},
		{ 0, (float)m_imageInfo.Height,0, 1, 0, 1, },
	};
	renderer.GetDevice()->SetTexture(0, m_texture);
	renderer.GetDevice()->SetVertexShader(NULL);
	renderer.GetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	renderer.GetDevice()->SetPixelShader(NULL);
	renderer.GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vertex, sizeof(TVERTEX));
}


void cTexture::CopyFrom(cTexture &src)
{
	if ((m_imageInfo.Height != src.m_imageInfo.Height)
		|| (m_imageInfo.Width != src.m_imageInfo.Width))
		return;

	D3DLOCKED_RECT slock;
	if (src.Lock(slock))
	{
		D3DLOCKED_RECT dlock;
		if (Lock(dlock))
		{
			memcpy(dlock.pBits, slock.pBits, slock.Pitch * src.m_imageInfo.Height);
			Unlock();
		}
		src.Unlock();
	}
}


void cTexture::CopyFrom(IDirect3DTexture9 *src)
{
	RET(!src);

	D3DSURFACE_DESC desc;
	if (FAILED(src->GetLevelDesc(0, &desc)))
		return;

	// Check If Texture Match Width - Height
	if ((m_imageInfo.Height != desc.Height)
		|| (m_imageInfo.Width != desc.Width))
		return;

	D3DLOCKED_RECT slock;
	if (SUCCEEDED(src->LockRect(0, &slock, NULL, 0)))
	{
		D3DLOCKED_RECT dlock;
		if (Lock(dlock))
		{
			memcpy(dlock.pBits, slock.pBits, slock.Pitch * m_imageInfo.Height);
			Unlock();
		}
		src->UnlockRect(0);
	}
}


bool cTexture::Lock(D3DLOCKED_RECT &out)
{
	RETV(!m_texture, false);
	m_texture->LockRect( 0, &out, NULL, 0 );
	return true;
}


void cTexture::Unlock()
{
	m_texture->UnlockRect( 0 );
}


void cTexture::Clear()
{
	m_fileName.clear();
	SAFE_RELEASE(m_texture);
}


// �ؽ��ĸ� ���Ͽ� �����Ѵ�.
bool cTexture::WritePNGFile( const string &fileName )
{
	if (FAILED(D3DXSaveTextureToFileA(fileName.c_str(), D3DXIFF_PNG, m_texture, NULL)))
	{
		return false;
	}
	return true;
}


// Render Text String on Texture
void cTexture::TextOut(cFontGdi &font, const string &text, const int x, const int y, const DWORD color)
{
	RET(!m_texture);

	IDirect3DSurface9* ppSurface = NULL;
	HDC mDC = NULL;
	if (m_texture->GetSurfaceLevel(0, &ppSurface) == D3D_OK)
	{
		if (ppSurface->GetDC(&mDC) == D3D_OK)
		{
			SelectObject(mDC, font.m_font);
			SetTextColor(mDC, color);
			SetBkMode(mDC, TRANSPARENT);
			TextOutA(mDC, x, y, text.c_str(), text.size());
			ppSurface->ReleaseDC(mDC);
		}
		ppSurface->Release();
	}
}


// Render Text String on Texture
void cTexture::DrawText(cFontGdi &font, const string &text, const sRect &rect, const DWORD color)
{
	RET(!m_texture);	

	IDirect3DSurface9* ppSurface = NULL;
	HDC mDC = NULL;
	if (m_texture->GetSurfaceLevel(0, &ppSurface) == D3D_OK)
	{
		if (ppSurface->GetDC(&mDC) == D3D_OK)
		{
			SelectObject(mDC, font.m_font);
			SetTextColor(mDC, color);
			SetBkMode(mDC, TRANSPARENT);
			::DrawTextA(mDC, text.c_str(), -1, (RECT*)&rect, DT_CENTER | DT_WORDBREAK);
			ppSurface->ReleaseDC(mDC);
		}
		ppSurface->Release();
	}
}


void cTexture::LostDevice()
{
	RET(!m_texture);
	SAFE_RELEASE(m_texture);
}


void cTexture::ResetDevice(cRenderer &renderer)
{
	SAFE_RELEASE(m_texture);

	if (m_fileName.empty())
	{
		Create(renderer, m_imageInfo.Width, m_imageInfo.Height, m_imageInfo.Format);
	}
	else
	{
		Create(renderer, m_fileName);	
	}
}
