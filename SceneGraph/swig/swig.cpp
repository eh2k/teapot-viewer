#include "swig.h"
#include <memory>
#include <map>
#include <..\src\IDriver.h>
#include <..\src\Viewport.h>
#include <..\src\Scene.h>
#include <..\src\SceneIO.h>
#include <..\src\Controller.h>

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <GL/gl.h>
#include "..\..\OpenGLDriver\GL\wglext.h"
#include "..\..\OpenGLDriver\GL\glext.h"

#pragma comment ( lib, "OPENGL32" )

namespace eh
{
	struct BaseViewPort : public IViewport
	{
		Ptr<Viewport> _viewPort;

		void setDisplayRect(int x, int y, int dx, int dy) override
		{
			_viewPort->setDisplayRect(x, y, dx, dy);
		}

		bool loadScene(std::wstring filePath, Callback* callback) override
		{
			SceneIO io;
			auto scene = Scene::create();
			if (io.read(filePath, scene, [callback](float value) { if(callback) callback->call(value); }))
			{
				_viewPort->setScene(scene, scene->createOrbitalCamera());
				_viewPort->invalidate();
				return true;
			}
			else
				return false;
		}

		IController* control() override
		{
			return &_viewPort->control();
		}

		bool isValid() override
		{
			return _viewPort->isValid();
		}

		std::string getDriverInfo() override
		{
			return _viewPort->getDriver()->getDriverInformation();
		}
	};

	struct D3DViewPort : public BaseViewPort
	{
		PIXELFORMATDESCRIPTOR m_pfd;
		HDC m_hdc;
		HWND m_hWnd;
		HGLRC m_hrc;
		GLenum	m_glError;

		void drawScene() override
		{
			_viewPort->drawScene();
		}

		D3DViewPort(HWND hWnd)
		{
			typedef IDriver* (*CreateDriverFunc)(void* pWindow);

			if (HMODULE hModule = LoadLibraryA("Direct3D9Driver.dll"))
			{
				auto CreateDriver = (CreateDriverFunc)GetProcAddress(hModule, "CreateDirect3D9Driver");

				_viewPort = new Viewport(CreateDriver(hWnd));
				_viewPort->setDisplayRect(0, 0, 1, 1);
				_viewPort->setScene(_viewPort->getScene(), _viewPort->getScene()->createOrbitalCamera());
			}
		}

		~D3DViewPort()
		{
		}
	};


	struct OpenGLViewPort : public BaseViewPort
	{
		PIXELFORMATDESCRIPTOR m_pfd;
		HDC m_hdc;
		HWND m_hWnd;
		HGLRC m_hrc;
		GLenum	m_glError;

		void drawScene() override
		{
			wglMakeCurrent(m_hdc, m_hrc);

			_viewPort->drawScene();

			SwapBuffers(m_hdc);
		}

		OpenGLViewPort(HWND hWnd) :m_hWnd(hWnd), m_hdc(::GetDC(hWnd))
		{
			//http://msdn.microsoft.com/en-us/library/ms970779.aspx
			//http://www.microsoft.com/msj/archive/S2085.aspx
			//http://www.3dsource.de/faq/mswindows.htm

			m_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);	// size of this pfd
			m_pfd.nVersion = 1;					// version numbe
			m_pfd.dwFlags = PFD_DRAW_TO_WINDOW |			// support window
				PFD_SUPPORT_OPENGL |			// support OpenGL
				PFD_DOUBLEBUFFER;			// double buffered
			m_pfd.iPixelType = PFD_TYPE_RGBA;	                // RGBA type
			m_pfd.cColorBits = 24;					// 24-bit color depth
			m_pfd.cRedBits = 0;
			m_pfd.cRedShift = 0;
			m_pfd.cGreenBits = 0;
			m_pfd.cGreenShift = 0;
			m_pfd.cBlueBits = 0;
			m_pfd.cBlueShift = 0;
			m_pfd.cAlphaBits = 8;					// 8-Bit alpha buffer
			m_pfd.cAlphaShift = 0;
			m_pfd.cAccumBits = 0;
			m_pfd.cAccumRedBits = 0;
			m_pfd.cAccumGreenBits = 0;
			m_pfd.cAccumBlueBits = 0;
			m_pfd.cAccumAlphaBits = 0;
			m_pfd.cDepthBits = 24;					// z-buffer
			m_pfd.cStencilBits = 8;					// stencil buffer
			m_pfd.cAuxBuffers = 0;					// no auxiliary buffer
			m_pfd.iLayerType = PFD_MAIN_PLANE;			// main layer
			m_pfd.bReserved = 0;
			m_pfd.dwLayerMask = 0;
			m_pfd.dwVisibleMask = 0;
			m_pfd.dwDamageMask = 0;

			DWORD typ = GetObjectType(m_hdc);

			if (typ == OBJ_DC)
				m_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			else
				m_pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL;

			int iPixelFormat = ChoosePixelFormat(m_hdc, &m_pfd);
			SetPixelFormat(m_hdc, iPixelFormat, &m_pfd);

			PIXELFORMATDESCRIPTOR  pfd2;
			DescribePixelFormat(m_hdc, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd2);

			DWORD dwRCMode = pfd2.dwFlags; // & PFD_GENERIC_FORMAT;

			try
			{
				m_hrc = wglCreateContext(m_hdc);

				if (m_hrc == NULL)
					throw 0;

				if (wglMakeCurrent(m_hdc, m_hrc) == FALSE)
					throw 0;

				m_glError = glGetError();

				if (m_glError != GL_NO_ERROR)
					throw m_glError;

			}
			catch (GLenum/* glError*/)
			{
				if (m_hrc)
				{
					wglMakeCurrent(m_hdc, NULL);
					wglDeleteContext(m_hrc);
					m_hrc = NULL;
				}
			}

			typedef IDriver* (*CreateDriverFunc)(void* pWindow);

			if (HMODULE hModule = LoadLibraryA("OpenGLDriver.dll"))
			{
				auto CreateDriver = (CreateDriverFunc)GetProcAddress(hModule, "CreateOpenGL1Driver");

				_viewPort = new Viewport(CreateDriver(hWnd));

				_viewPort->setScene(_viewPort->getScene(), _viewPort->getScene()->createOrbitalCamera());
			}
		}

		~OpenGLViewPort()
		{
			auto scene = Scene::create();
			_viewPort->setScene(scene, scene->createOrbitalCamera());
		}
	};

	std::shared_ptr<IViewport> CreateViewport(void* hWindow)
	{
		if (::IsWindow((HWND)hWindow) == false)
			return nullptr;

		return std::make_shared<D3DViewPort>((HWND)hWindow);
	}
}