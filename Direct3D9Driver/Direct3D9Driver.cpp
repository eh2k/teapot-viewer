/****************************************************************************
* Copyright (C) 2007-2010 by E.Heidt  http://teapot-viewer.sourceforge.net/ *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
****************************************************************************/

#include "..\SceneGraph\src\IDriver.h"
#include "..\SceneGraph\src\Geometry.h"
#include "..\SceneGraph\src\Material.h"
#include "..\SceneGraph\src\SceneIO.h"

using namespace eh;

#if defined(_DEBUG)
#define D3D_DEBUG_INFO
#endif

#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>

#include <d3d9.h>
#include <D3dx9core.h>
#include <D3dx9math.h>
//#include <DXErr.h>

static HRESULT DXGetErrorStringW(HRESULT result)
{
	return result;
}

//#include <atlstr.h>
#include <fstream>

#pragma comment ( lib, "d3d9" )
#pragma comment ( lib, "d3dx9" )
#pragma comment ( lib, "dxerr" )

static_assert(sizeof(Matrix) == sizeof(D3DMATRIX), "");
static_assert(sizeof(Vec3) == sizeof(D3DVECTOR), "");
static_assert(sizeof(RGBA) == sizeof(D3DCOLORVALUE), "");

static UINT video_mem = 0;

class PerformanceCouter
{
private:
	__int64  timerFreq, startTime, endTime;  // Start and ends times
	float    m_fps;
public:
	PerformanceCouter():m_fps(0){}
	~PerformanceCouter(){}
	inline void start()
	{
		QueryPerformanceFrequency((LARGE_INTEGER *)&timerFreq);
		QueryPerformanceCounter((LARGE_INTEGER *)&startTime);
	}

	inline void stop()
	{
		QueryPerformanceCounter((LARGE_INTEGER *)&endTime);
		float fps = (float)((double)timerFreq / ((double)(endTime - startTime)));

		if(m_fps < 1)
			m_fps = fps;
		else
			m_fps = (m_fps + fps)/2;
	}

	inline float getFPS(){return m_fps;}
	inline float getMS(){return 0.01f/m_fps;}
};

static PerformanceCouter s_fps;
static Uint s_mat_changes = 0;
static Uint s_buf_changes = 0;
static Uint s_vbc = 0;
static Uint s_ibc = 0;


template< class T>
inline void SAFE_RELEASE(T*& i) { if(i) i->Release(); i=NULL;}

class Direct3D9VertexBuffer: public IResource
{
public:
	static Ptr<Direct3D9VertexBuffer> create(IDirect3DDevice9* m_pDevice, Ptr<IVertexBuffer> pBuff)
	{
		if(pBuff->getBufferSize() == 0)
			return nullptr;

		Ptr<Direct3D9VertexBuffer> ret = pBuff->m_resource;

		if( ret )
			return ret;

		UINT stride = pBuff->getStride();
		DWORD FVF = 0;

		if(stride == sizeof(Vec3))
			FVF = D3DFVF_XYZ;
		else if(stride == sizeof(Vec3)*2)
			FVF = D3DFVF_XYZ|D3DFVF_NORMAL;
		else if(stride == sizeof(Vec3)*2+sizeof(Float)*2)
			FVF = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE2(0);
		else
			assert(0); //darf nicht sein!!

		IDirect3DVertexBuffer9* pVB = NULL;

		HRESULT hr = m_pDevice->CreateVertexBuffer(pBuff->getBufferSize(), 
			D3DUSAGE_WRITEONLY, 
			FVF, 
			D3DPOOL_MANAGED,
			&pVB, 
			NULL);

		if( SUCCEEDED(hr) && pVB)
		{
			void* vData = NULL;
			if( SUCCEEDED( pVB->Lock(0, pBuff->getBufferSize(), &vData, 0) ))
			{
				memcpy(vData, pBuff->getBuffer(), pBuff->getBufferSize());

				// Hier wird die Texture Y Koordinate umgerechnet ...
				if(stride == sizeof(Vec3)*2+sizeof(Float)*2)
				{
					Float* p = (Float*)vData;
					for(size_t i = 7; i < pBuff->getBufferSize()/4; i+= 8)
						p[i] = 1.f - p[i];
				}

				if(FAILED(pVB->Unlock()))
					return nullptr;
			}
			else
				return nullptr;

			ret = new Direct3D9VertexBuffer(pVB, FVF, stride);
			pBuff->m_resource = ret;
			return ret; 
		}
		else
		{
			return nullptr;
		}
	}

	bool bind(IDirect3DDevice9* m_pDevice)
	{
		return SUCCEEDED(m_pDevice->SetFVF( m_FVF )) &&
			SUCCEEDED(m_pDevice->SetStreamSource( 0, m_pVB, 0, m_stride ));
	}

	UINT getSize() const
	{
		D3DVERTEXBUFFER_DESC desc;
		m_pVB->GetDesc(&desc);
		return desc.Size;
	}

	virtual ~Direct3D9VertexBuffer()
	{
		video_mem -= getSize();
		s_vbc--;
		SAFE_RELEASE(m_pVB);
	}

protected:
	Direct3D9VertexBuffer(IDirect3DVertexBuffer9* pVB, DWORD FVF, UINT stride):
		m_pVB(pVB),m_FVF(FVF),m_stride(stride)
	{
		video_mem += getSize();
		s_vbc++;
	}
private:
	UINT m_stride;
	DWORD m_FVF;
	IDirect3DVertexBuffer9* m_pVB;
};

class Direct3D9IndexBuffer: public IResource
{
public:
	static Ptr<IResource> create(IDirect3DDevice9* m_pDevice, const Uint_vec& indices)
	{
		if(indices.size() == 0)
			return nullptr;
		else
		{
			IDirect3DIndexBuffer9* pIB = NULL;

			HRESULT hr = m_pDevice->CreateIndexBuffer(  sizeof(indices[0])*(UINT)indices.size(), 
				D3DUSAGE_WRITEONLY, 
				D3DFMT_INDEX32, 
				D3DPOOL_MANAGED, 
				&pIB, 
				NULL);

			if( SUCCEEDED(hr) && pIB)
			{
				void* vData = NULL;
				if( SUCCEEDED( pIB->Lock(0, sizeof(indices[0])*(UINT)indices.size(), &vData, 0) ))
				{
					memcpy(vData, &indices[0], sizeof(indices[0])*indices.size());
					if(FAILED(pIB->Unlock()))
						return nullptr;
				}
				else
					return nullptr;

				return new Direct3D9IndexBuffer(pIB);
			}
			else
			{
				return nullptr;
			}
		}
	}

	bool bind(IDirect3DDevice9* m_pDevice)
	{
		return SUCCEEDED(m_pDevice->SetIndices( m_pIB ));
	}

	UINT getSize() const
	{
		D3DINDEXBUFFER_DESC desc;
		m_pIB->GetDesc(&desc);
		return desc.Size;
	}

	virtual ~Direct3D9IndexBuffer()
	{
		video_mem -= getSize();
		s_ibc--;
		SAFE_RELEASE(m_pIB);
	}

protected:
	Direct3D9IndexBuffer(IDirect3DIndexBuffer9* pIB):m_pIB(pIB)
	{
		video_mem += getSize();
		s_ibc++;
	};
private:
	IDirect3DIndexBuffer9* m_pIB;
};

class Direct3D9Texture: public IResource
{
public:
	static Ptr<Direct3D9Texture> create(IDirect3DDevice9* m_pDevice, const std::wstring& sFile)
	{
		SceneIO::File aFile(sFile);
		SceneIO::setStatusText( std::wstring(L"Laoding ") + aFile.getName() + L"...");

		std::unique_ptr<char> data;
		size_t size = aFile.getContent(data);

		if( size == 0 )
		{
			std::wcerr << aFile.getPath().c_str() << " SceneIO::File::getContent(data.get()) failed" << std::endl;
			return nullptr;
		}

		IDirect3DTexture9* pTexture = NULL;
		if(SUCCEEDED(::D3DXCreateTextureFromFileInMemory( m_pDevice, data.get(), size, &pTexture )))
			return new Direct3D9Texture(pTexture);
		else
		{
			std::wcerr << L"Direct3D9Texture::create failed: " << sFile.c_str() << std::endl;;
			return nullptr;
		}
	}

	IDirect3DBaseTexture9* get()
	{
		return m_pTexture;
	}

	virtual ~Direct3D9Texture()
	{
		SAFE_RELEASE(m_pTexture);
	};
private:
	Direct3D9Texture(IDirect3DBaseTexture9* pTexture):
		m_pTexture(pTexture)
	{
	}

	IDirect3DBaseTexture9* m_pTexture;
};

class Direct3D9Driver: public IDriver
{
	static unsigned int nRefCount;
	static IDirect3DDevice9* m_pDevice;
	static ID3DXFont* m_pFont;
	static D3DCAPS9 caps;
	static D3DADAPTER_IDENTIFIER9 adapter;

	static ID3DXEffect* m_pShader;
	static UINT	    cEffectPasses;

	IDirect3DSwapChain9* pSwapChain;
	D3DPRESENT_PARAMETERS present; 
	Rect	 m_viewport;

	bool	m_bShadow;
	HRESULT hr;
	HWND hWnd;
	Uint nAntialiasingLevel;

	UINT nFrameBufferWidth;
	UINT nFrameBufferHeight;

public:
	Direct3D9Driver(HWND _hWnd):
		hWnd(_hWnd),
		m_bShadow(true),
		pSwapChain(NULL),
		hr(0),
		nAntialiasingLevel(8),
		m_viewport()
	{
		assert(::IsWindow(hWnd));

		nRefCount++;

		ZeroMemory( &present, sizeof(present) );

		IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
		if( d3d9 )
		{
			D3DDISPLAYMODE mode;
			if(SUCCEEDED(d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT , &mode)) ) 
			{
				this->present.Windowed			= TRUE;
				this->present.SwapEffect		= D3DSWAPEFFECT_DISCARD;

				for(; nAntialiasingLevel >= 0; nAntialiasingLevel--)
					if(isMultiSampleSupported(nAntialiasingLevel, 
						0,
						d3d9, 
						&this->present.MultiSampleType, 
						&this->present.MultiSampleQuality))
						break;

				this->present.BackBufferFormat		= mode.Format;

				this->present.BackBufferWidth		= nFrameBufferWidth  = mode.Width;
				this->present.BackBufferHeight		= nFrameBufferHeight = mode.Height;

				this->present.hDeviceWindow		= hWnd;
				this->present.AutoDepthStencilFormat	= D3DFMT_D24S8;
				this->present.EnableAutoDepthStencil	= TRUE;
				this->present.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;

				if(this->m_pDevice == NULL)
				{
					ZeroMemory( &caps, sizeof(caps) );
					ZeroMemory( &adapter, sizeof(adapter));

					d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &this->caps);
					d3d9->GetAdapterIdentifier(D3DADAPTER_DEFAULT,0, &adapter);

					INT32 flags = 0;
					if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
						flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
					else
						flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

					hr = d3d9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
						flags| D3DCREATE_FPU_PRESERVE, 
						&this->present, &this->m_pDevice);

					if( FAILED( hr ) ) 
					{
						SAFE_RELEASE(m_pDevice);
						SAFE_RELEASE(d3d9);
						return;
					}

					hr = D3DXCreateFontW( this->m_pDevice, 16, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &this->m_pFont );

					this->m_pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );

					//this->m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
					this->m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

					this->m_pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
					this->m_pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

					loadShader();

					this->m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
					this->m_pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
					this->m_pDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
					this->m_pDevice->SetRenderState( D3DRS_LOCALVIEWER, TRUE );

					this->m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					this->m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);


				}

				RECT r; 
				if(GetWindowRect(hWnd, &r))
				{
					this->present.BackBufferWidth	= r.right-r.left;
					this->present.BackBufferHeight	= r.bottom-r.top;

					if(this->present.BackBufferWidth == 0 || this->present.BackBufferHeight == 0)
					{
						this->present.BackBufferWidth = 640;
						this->present.BackBufferHeight = 480;
					}
				}

				hr = this->m_pDevice->CreateAdditionalSwapChain(&this->present, &this->pSwapChain);
			}

			SAFE_RELEASE(d3d9); // done with d3d9 object

		}
		else
			std::cerr << "Direct3DCreate9 failed." << std::endl;
	}
	virtual ~Direct3D9Driver()
	{
		SAFE_RELEASE(pSwapChain);

		if(--nRefCount==0)
		{
			SAFE_RELEASE(m_pFont);
			SAFE_RELEASE(m_pDevice);
			SAFE_RELEASE(m_pShader);
		}
	}

	virtual bool loadShader()
	{
		if( caps.VertexShaderVersion >= D3DVS_VERSION(1,1) )
		{
			LPD3DXBUFFER pBufferErrors = NULL;

			ID3DXEffect* pShader = NULL;

			HMODULE hModule = GetModuleHandleW(L"DIRECT3D9DRIVER.DLL");
			assert(hModule);
			hr = D3DXCreateEffectFromResourceW(this->m_pDevice, 
				hModule,
				L"RC_SHADER",
				NULL,
				NULL,
				0, //D3DXFX_NOT_CLONEABLE,
				NULL,
				&pShader,
				&pBufferErrors);

			if( FAILED( hr ))
			{
				if(pBufferErrors)
					std::cerr << "Shader Error: " << pBufferErrors->GetBufferPointer() << std::endl;

				return false;
			}

			SAFE_RELEASE(this->m_pShader);
			this->m_pShader = pShader;

		}	
		return true;
	}

	inline bool verifyNoErrors() const
	{
		if(this->m_pDevice && this->m_pFont && this->pSwapChain /*&& this->m_pShader*/ )
			return true;
		else
		{
			return false;
		}
	}

	bool isMultiSampleSupported(int nSamples, 
		DWORD nMaxQuality,
		IDirect3D9* d3d9, 
		D3DMULTISAMPLE_TYPE* pType, 
		DWORD* pMultiSampleQuality)
	{
		switch(nSamples)
		{
		default:
			*pType = D3DMULTISAMPLE_NONE;	   break;
			//case 1: *pType = D3DMULTISAMPLE_NONMASKABLE; break;
		case 2: *pType = D3DMULTISAMPLE_2_SAMPLES; break;
		case 3: *pType = D3DMULTISAMPLE_3_SAMPLES; break;
		case 4: *pType = D3DMULTISAMPLE_4_SAMPLES; break;
		case 5: *pType = D3DMULTISAMPLE_5_SAMPLES; break;
		case 6: *pType = D3DMULTISAMPLE_6_SAMPLES; break;
		case 7: *pType = D3DMULTISAMPLE_7_SAMPLES; break;
		case 8: *pType = D3DMULTISAMPLE_8_SAMPLES; break;
		case 9: *pType = D3DMULTISAMPLE_9_SAMPLES; break;
		case 10: *pType = D3DMULTISAMPLE_10_SAMPLES; break;
		case 11: *pType = D3DMULTISAMPLE_11_SAMPLES; break;
		case 12: *pType = D3DMULTISAMPLE_12_SAMPLES; break;
		case 13: *pType = D3DMULTISAMPLE_13_SAMPLES; break;
		case 14: *pType = D3DMULTISAMPLE_14_SAMPLES; break;
		case 15: *pType = D3DMULTISAMPLE_15_SAMPLES; break;
		case 16: *pType = D3DMULTISAMPLE_16_SAMPLES; break;
		}

		bool bRet = false;

		DWORD dwQuality = 0;

		if( SUCCEEDED(d3d9->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, D3DFMT_D24S8, TRUE, 
			*pType, &dwQuality ) ) )
		{
			if(dwQuality <= nMaxQuality)
				*pMultiSampleQuality = dwQuality-1;
			else
				*pMultiSampleQuality = nMaxQuality;

			bRet = true;
		}
		else
		{
			*pType = D3DMULTISAMPLE_NONE;
			bRet = false;
		}

		return bRet;
	}



	virtual std::string getDriverInformation() const
	{
		if(verifyNoErrors() == false)
			return "";

		int Product = HIWORD(this->adapter.DriverVersion.HighPart);
		int Version = LOWORD(this->adapter.DriverVersion.HighPart);
		int SubVersion = HIWORD(this->adapter.DriverVersion.LowPart);
		int Build = LOWORD(this->adapter.DriverVersion.LowPart);

		std::stringstream str;

		str << "Description: "  << (const char*)this->adapter.Description << std::endl;
		str << "Driver: "	<< (const char*)this->adapter.Driver << std::endl;
		str << "Version: "	<< Product << "." << Version << "." << SubVersion << "." << Build << std::endl;

		return str.str();
	}

	virtual void enableDepthTest(bool bEnable)
	{
		this->m_pDevice->SetRenderState( D3DRS_ZENABLE, bEnable?D3DZB_TRUE:D3DZB_FALSE );
	}

	virtual bool beginScene(bool bDrawBG)
	{
		if(verifyNoErrors() == false)
			return false;

		s_fps.start();
		s_mat_changes = 0;
		s_buf_changes = 0;


		HRESULT hr = 0;

		IDirect3DSurface9* pDestSurface = NULL;
		hr = this->pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pDestSurface);

		if(FAILED(hr))
			std::wcerr << L"IDirect3DDevice9::beginScene failed. Reason: " <<  DXGetErrorStringW(hr) << std::endl;

		hr = this->m_pDevice->SetRenderTarget(0, pDestSurface);

		pDestSurface->Release();

		if(FAILED(hr))
			std::wcerr << L"IDirect3DDevice9::beginScene failed. Reason: " << DXGetErrorStringW(hr) << std::endl;

		hr = this->m_pDevice->Clear( 0L, NULL, 
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 
			D3DCOLOR_RGBA(0xFF,0xFF,0xFF,0), 1.0f, 0 );

		if(FAILED(hr))
			std::wcerr << L"IDirect3DDevice9::beginScene failed. Reason: " << DXGetErrorStringW(hr) << std::endl;

		hr = this->m_pDevice->BeginScene();

		if(FAILED(hr))
			std::wcerr << L"IDirect3DDevice9::beginScene failed. Reason: " << DXGetErrorStringW(hr) << std::endl;

		if(this->m_pShader)
		{	
			this->m_pShader->Begin( &this->cEffectPasses, 0 );

			this->setTexture(NULL, 0);
			this->setTexture(NULL, 1);
			this->setTexture(NULL, 2);
			this->setTexture(NULL, 3);
			this->enableWireframe(false);
			this->enableLighting(false);

			if(bDrawBG)
			{
				this->enableZWriting(false);
				this->setProjectionMatrix( Matrix::Ortho(0, 1.f, -1.f, 1.f, -1.f, +1.f) );
				this->setViewMatrix( Matrix::Identity() );
				this->setWorldMatrix( Matrix::Identity() );

				FLOAT disable[] = { -1.f, -1.f, -1.f, -1.f };

				this->m_pShader->SetFloatArray( "rgbaColor", disable, 4 );

				this->m_pShader->BeginPass( 0 );

				this->m_pDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);

				std::pair< Vec3, DWORD > data[4];

				data[0].first = Vec3(0.f, 0.f, 0.f); data[0].second = 0x00EEEEEE;
				data[1].first = Vec3(1.f, 0.f, 0.f); data[1].second = 0x00EEEEEE;
				data[2].first = Vec3(1.f, 1.f, 0.f); data[2].second = 0x00EEEEFF;
				data[3].first = Vec3(0.f, 1.f, 0.f); data[3].second = 0x00EEEEFF;

				this->m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &data[0], sizeof(data[0]));

				data[0].first = Vec3(0.f, -1.f, 0.f); data[0].second = 0x00555555;
				data[1].first = Vec3(1.f, -1.f, 0.f); data[1].second = 0x00555555;
				data[2].first = Vec3(1.f, 0.f, 0.f); data[2].second = 0x00EEEEEE;
				data[3].first = Vec3(0.f, 0.f, 0.f); data[3].second = 0x00EEEEEE;

				this->m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &data[0], sizeof(data[0]));

				this->m_pShader->EndPass();

			}

			this->enableZWriting(true);
		}

		return SUCCEEDED(hr);
	}

	virtual bool endScene( bool bShowFPS )
	{
		if(verifyNoErrors() == false)
			return false;

		s_fps.stop();

		std::wostringstream str;
		str << L" fps: " << s_fps.getFPS();
		//		str << " mc: " << s_mat_changes;
		//		str << " bc: " << s_buf_changes;
		//		str << " vbc: " << s_vbc;
		//		str << " ibc: " << s_ibc;
		//		str << " VBcount: " << IVertexBuffer::m_nCount;
		//		str << " RessMemSize: " << getResourceMemorySize()/1024 << " KB";

		if(bShowFPS)
			SceneIO::setStatusText(str.str());//, (int)this->getViewport().Width()-80, 10);

		if(this->m_pShader)
			this->m_pShader->End();

		return SUCCEEDED(this->m_pDevice->EndScene()) && SUCCEEDED(this->pSwapChain->Present(0,0,0,0,0));
	}

	virtual void setViewport(int x, int y, int dx, int dy)
	{
		if(verifyNoErrors() == false)
			return;

		if(nFrameBufferWidth < (Uint)(dx-x) || nFrameBufferHeight < (Uint)(dy-y))
		{
			assert(0);	//Warum?
			return;
		}

		m_viewport = Rect((Float)x,(Float)y,(Float)dx,(Float)dy);

		assert(m_viewport.Top() < m_viewport.Bottom());

		D3DVIEWPORT9 viewport; ZeroMemory(&viewport, sizeof(viewport));
		if(FAILED(this->m_pDevice->GetViewport(&viewport)))
			std::wcerr << L"GetViewport failed." << std::endl;

		viewport.X = x;
		viewport.Y = y;
		viewport.Width = dx-x;
		viewport.Height = dy-y;

		if(FAILED(this->m_pDevice->SetViewport(&viewport)))
			std::wcerr << L"SetViewport failed." << std::endl;

		if( this->present.BackBufferWidth != viewport.Width ||
			this->present.BackBufferHeight != viewport.Height )
		{
			this->present.BackBufferWidth  = viewport.Width;
			this->present.BackBufferHeight = viewport.Height;

			SAFE_RELEASE(this->pSwapChain);

			HRESULT hr = this->m_pDevice->CreateAdditionalSwapChain(&this->present, &this->pSwapChain);

			if(FAILED(hr))
				std::wcerr << L"IDirect3DDevice9::CreateAdditionalSwapChain failed. Reason: " << DXGetErrorStringW(hr) << std::endl;
		}
	}

	virtual void enableWireframe(bool bEnable)
	{
		if(verifyNoErrors() == false)
			return;

		if(bEnable)
			this->m_pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		else
			this->m_pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}

	virtual const Rect& getViewport() const 
	{ 
		assert(m_viewport.Top() < m_viewport.Bottom());

		return m_viewport; 
	}

	virtual void setProjectionMatrix(const Matrix& mat)
	{
		if(this->m_pShader)
			this->m_pShader->SetMatrix( "matProj", reinterpret_cast<const D3DXMATRIX*>(&mat) );
		else
			this->m_pDevice->SetTransform(D3DTS_PROJECTION, reinterpret_cast<const D3DMATRIX*>(&mat));

	}

	virtual void setViewMatrix(const Matrix& mat)
	{
		if(this->m_pShader)
			this->m_pShader->SetMatrix( "matView", reinterpret_cast<const D3DXMATRIX*>(&mat) );
		else
			this->m_pDevice->SetTransform(D3DTS_VIEW, reinterpret_cast<const D3DMATRIX*>(&mat));
	}

	virtual void setWorldMatrix(const Matrix& mat)
	{
		if(this->m_pShader)
		{
			Matrix proj(false);  
			this->m_pShader->GetMatrix("matProj", reinterpret_cast<D3DXMATRIX*>(&proj));

			Matrix view(false);  
			this->m_pShader->GetMatrix("matView",reinterpret_cast<D3DXMATRIX*>(&view));

			this->m_pShader->SetMatrix( "matWorld", reinterpret_cast<const D3DXMATRIX*>(&mat) );
			this->m_pShader->SetMatrix( "matWorldViewProj", &reinterpret_cast<D3DXMATRIX&>(mat * view * proj) );

			// Calculate the transpose of the inverse of the world matrix.
			static D3DXMATRIX worldInverseTransposeMatrix;
			D3DXMatrixInverse(&worldInverseTransposeMatrix, 0, &reinterpret_cast<const D3DXMATRIX&>(mat*view));
			D3DXMatrixTranspose(&worldInverseTransposeMatrix, &worldInverseTransposeMatrix);
			this->m_pShader->SetMatrix( "matWorldInverseTranspose", &worldInverseTransposeMatrix );
		}
		else
			this->m_pDevice->SetTransform(D3DTS_WORLD, reinterpret_cast<const D3DMATRIX*>(&mat));
	}

	virtual void setShadowMatrix(const Matrix& mat)
	{
		if(this->m_pShader)
			this->m_pShader->SetMatrix( "matShadow", reinterpret_cast<const D3DXMATRIX*>(&mat) );
	}

	virtual void enableBlending(bool bEnable)
	{
		if(bEnable)
		{
			this->m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);		//glEnable(GL_BLEND);
		}
		else
		{
			this->m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);	//glDisable(GL_BLEND);
		}
	}
	virtual void enableZWriting(bool bEnable)
	{
		if(bEnable)
			this->m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE );	//glDepthMask(true);
		else
			this->m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE );	//glDepthMask(false);
	}
	virtual void enableShadow(bool bEnable)
	{
		m_bShadow = bEnable;
	}
	virtual void enableLighting(bool bEnable)
	{
		if(this->m_pShader)
			this->m_pShader->SetBool( "bLighting", (BOOL)bEnable );
	}
	virtual void enableCulling(bool bEnable)
	{
		if(bEnable)
			this->m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
		else
			this->m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	}
	virtual void cullFace(bool bEnable)
	{
		if(bEnable)
			this->m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );	//glCullFace(GL_FRONT);)
		else
			this->m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );		//glCullFace(GL_BACK);
	}

	virtual void setDepthOffset(Uint n, Float f)
	{
		D3DVIEWPORT9 viewport;
		this->m_pDevice->GetViewport(&viewport);

		viewport.MinZ = 5*f-(f*n);
		viewport.MaxZ = 1 - (f*n);

		this->m_pDevice->SetViewport(&viewport);
	}

	virtual void setMaterial(const Material* pMaterial)
	{
		s_mat_changes++;

		if(this->m_pShader)
			this->m_pShader->SetFloatArray( "rgbaColor", pMaterial->getDiffuse(), 4 );
		else
		{
			D3DMATERIAL9 mat; memset(&mat, 0, sizeof(mat));
			mat.Diffuse = reinterpret_cast<const D3DCOLORVALUE&>(pMaterial->getDiffuse());
			this->m_pDevice->SetMaterial(&mat);
		}

		setTexture( pMaterial->getTexture().get(), 0);
		setTexture( pMaterial->getReflTexture().get(), 1);
		setTexture( pMaterial->getBumpTexture().get(), 2);
		setTexture( pMaterial->getOpacTexture().get(), 3);

	}

	virtual void setTexture(Texture* pTexture, int id)
	{
		if(this->m_pShader == NULL)
			return;

		const char* tID = NULL;
		const char* bID = NULL;
		switch(id)
		{
		case 0: tID = "texDefault"; bID = "bEnableTexDefault"; break;
		case 1: tID = "texReflect"; bID = "bEnableTexReflect"; break;
		case 2: tID = "texBump"; bID = "bEnableTexBump"; break;
		case 3: tID = "texOpac"; bID = "bEnableTexOpac"; break;
		};

		if(pTexture)
		{
			Ptr<Direct3D9Texture> texture = pTexture->m_resource;

			if(texture == nullptr)
				texture = pTexture->m_resource = Direct3D9Texture::create(this->m_pDevice, pTexture->getFile() );

			if(texture)
			{
				this->m_pShader->SetBool( bID, true );
				this->m_pShader->SetTexture( tID, texture->get() );
			}
		}
		else
		{
			this->m_pShader->SetBool( bID, false );
		}
	}

	virtual bool drawPrimitive(Geometry& node)
	{
		Ptr<Direct3D9IndexBuffer> pIB = node.m_resource;
		Ptr<Direct3D9VertexBuffer> pVB = node.getVertexBuffer()->m_resource;

		if( pVB == nullptr)
			pVB = node.getVertexBuffer()->m_resource = Direct3D9VertexBuffer::create(this->m_pDevice, node.getVertexBuffer() );

		if( pIB == nullptr)
			pIB = node.m_resource = Direct3D9IndexBuffer::create(this->m_pDevice, node.getIndices());

		if( pVB && pIB && pVB->bind(this->m_pDevice) && pIB->bind(this->m_pDevice))
		{
			UINT nPrimitives = 0;

			if(node.getIndices().size()>0)
				nPrimitives = (UINT) node.getIndices().size();
			else
				nPrimitives = node.getVertexBuffer()->getVertexCount();

			D3DPRIMITIVETYPE mode = D3DPT_TRIANGLELIST;
			switch(node.getType())
			{
			case Geometry::POINTS:		
				mode = D3DPT_POINTLIST;	
				break;
			case Geometry::LINES:		
				mode = D3DPT_LINELIST;		
				nPrimitives /= 2;
				break;
			case Geometry::LINE_STRIP:		
				mode = D3DPT_LINESTRIP;	
				nPrimitives = nPrimitives-1;
				break;
			case Geometry::TRIANGLES:		
				mode = D3DPT_TRIANGLELIST;
				nPrimitives /= 3;
				break;
			case Geometry::TRIANGLE_STRIP:	
				mode = D3DPT_TRIANGLESTRIP;	
				nPrimitives = nPrimitives-2;
				break;
			case Geometry::TRIANGLE_FAN:	
				mode = D3DPT_TRIANGLEFAN;	
				nPrimitives = nPrimitives-2;
				break;
			default:
				std::wcerr << L"Unknown D3DPRIMITIVETYPE" << std::endl;
				break;
			}

			for(UINT iPass = 0; iPass < this->cEffectPasses; iPass++ )
			{	
				if(this->m_pShader)
					this->m_pShader->BeginPass( iPass );

				if(node.getIndices().size()==0)
				{
					this->m_pDevice->DrawPrimitive( mode, 0, nPrimitives );
					//					this->m_pDevice->DrawPrimitiveUP( mode, nPrimitives, node.getVertexBuffer()->getBuffer(), node.getVertexBuffer()->getStride() );
				}
				else
				{
					this->m_pDevice->DrawIndexedPrimitive(mode, 0, 0, node.getVertexBuffer()->getVertexCount(), 0, nPrimitives);
					//					this->m_pDevice->DrawIndexedPrimitiveUP(mode, 0, node.getVertexBuffer()->getVertexCount(), nPrimitives, &node.getIndices()[0], D3DFMT_INDEX32, node.getVertexBuffer()->getBuffer(), node.getVertexBuffer()->getStride() );
				}

				if(this->m_pShader)
					this->m_pShader->EndPass();

				if(!m_bShadow)	//Kein Shadow pass zeichnen...
					break;
			}

			return true;
		}

		return false;
	}

	virtual void draw2DText(const char* text, int x, int y)
	{
		RECT rc;
		rc.left   = x;
		rc.bottom = (LONG)m_viewport.Height()-y;
		rc.right  = 0;
		rc.top    = 0;
		this->m_pFont->DrawTextA( NULL, text, -1, &rc, DT_NOCLIP|DT_BOTTOM,  D3DXCOLOR( 0, 0, 0, 1 ) );
	}
};

unsigned int		Direct3D9Driver::nRefCount = 0;
IDirect3DDevice9*	Direct3D9Driver::m_pDevice = NULL;
ID3DXFont*		Direct3D9Driver::m_pFont = NULL;
D3DADAPTER_IDENTIFIER9	Direct3D9Driver::adapter;
D3DCAPS9		Direct3D9Driver::caps;
ID3DXEffect*		Direct3D9Driver::m_pShader = NULL;
UINT			Direct3D9Driver::cEffectPasses = 0;

extern "C" __declspec(dllexport) IDriver* CreateDirect3D9Driver(int* hWnd)
{
	Direct3D9Driver* pDriver = new Direct3D9Driver((HWND)hWnd);
	if(pDriver->verifyNoErrors() == false)
	{
		delete pDriver;
		return NULL;
	}
	return pDriver;
}
