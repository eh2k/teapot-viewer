#include "swig.h"
#include <memory>
#include <map>
#include <..\src\IDriver.h>
#include <..\src\Viewport.h>
#include <..\src\Scene.h>
#include <..\src\SceneIO.h>
#include <..\src\Controller.h>
#include <..\src\VertexBuffer.h>

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <GL/gl.h>
#include "..\..\OpenGLDriver\GL\wglext.h"
#include "..\..\OpenGLDriver\GL\glext.h"

#pragma comment ( lib, "OPENGL32" )

namespace swig
{
	struct BaseViewPort : public IViewport
	{
		eh::Ptr<eh::Viewport> _viewPort;

		void SetDisplayRect(int x, int y, int dx, int dy) override
		{
			_viewPort->setDisplayRect(x, y, dx, dy);
		}

		IController* Control() override
		{
			return &_viewPort->control();
		}

		bool IsValid() override
		{
			return _viewPort->isValid();
		}

		std::string GetDriverInfo() override
		{
			return _viewPort->getDriver()->getDriverInformation();
		}

		void SetModeFlag(Mode flag, bool enable) override
		{
			_viewPort->setModeFlag(flag, enable);
			_viewPort->invalidate();
		}

		bool GetModeFlag(Mode flag) override
		{
			return _viewPort->getModeFlag(flag);
			_viewPort->invalidate();
		}

		int GetCameraCount() override
		{
			return (int)_viewPort->getScene()->getCameras().size() + 1;
		}

		std::wstring GetCameraName(int num)
		{
			auto name = _viewPort->getScene()->getCameras()[num - 1]->getName();
			return std::wstring(name.cbegin(), name.cend());
		}

		void SetCamera(int num) override
		{
			if (num == 0)
			{
				_viewPort->setScene(_viewPort->getScene(), _viewPort->getScene()->createOrbitalCamera());
			}
			else
			{
				_viewPort->setScene(_viewPort->getScene(), _viewPort->getScene()->getCameras()[num - 1]);
			}

			_viewPort->invalidate();
		}

		void SetScene(std::shared_ptr<ISceneNode> scene) override;
	};

	struct D3DViewPort : public BaseViewPort
	{
		PIXELFORMATDESCRIPTOR m_pfd;
		HDC m_hdc;
		HWND m_hWnd;
		HGLRC m_hrc;
		GLenum	m_glError;

		void DrawScene() override
		{
			_viewPort->drawScene();
		}

		D3DViewPort(HWND hWnd)
		{
			typedef eh::IDriver* (*CreateDriverFunc)(void* pWindow);

			if (HMODULE hModule = LoadLibraryA("Direct3D9Driver.dll"))
			{
				auto CreateDriver = (CreateDriverFunc)GetProcAddress(hModule, "CreateDirect3D9Driver");

				_viewPort = new eh::Viewport(CreateDriver(hWnd));
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

		void DrawScene() override
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

			typedef eh::IDriver* (*CreateDriverFunc)(void* pWindow);

			if (HMODULE hModule = LoadLibraryA("OpenGLDriver.dll"))
			{
				auto CreateDriver = (CreateDriverFunc)GetProcAddress(hModule, "CreateOpenGL1Driver");

				_viewPort = new eh::Viewport(CreateDriver(hWnd));

				_viewPort->setScene(_viewPort->getScene(), _viewPort->getScene()->createOrbitalCamera());
			}
		}

		~OpenGLViewPort()
		{
			auto scene = eh::Scene::create();
			_viewPort->setScene(scene, scene->createOrbitalCamera());
		}
	};

	std::shared_ptr<IViewport> CreateViewport(void* hWindow)
	{
		if (::IsWindow((HWND)hWindow) == false)
			return nullptr;

		return std::make_shared<D3DViewPort>((HWND)hWindow);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	struct Geometry : public IGeometry
	{
		eh::Ptr<eh::IVertexBuffer> _p = nullptr;
		eh::Uint_vec _indices;

		Geometry(eh::Ptr<eh::IVertexBuffer> p) : _p(p)
		{}

		void AddVertex(const math3D::Vec3& p, const math3D::Vec3& n, const math3D::Vec3& t) override
		{
			_indices.push_back(_p->addVertex(p, n, t));
		}
	};

	struct Material : public IMaterial
	{
		eh::Ptr<eh::Material> _p = nullptr;

		Material(eh::Ptr<eh::Material> p) : _p(p)
		{}

		void SetDiffuseColor(float r, float g, float b, float a) override
		{
			_p->setDiffuse(eh::RGBA(r, g, b, a));
		}

		virtual void SetAmbientColor(float r, float g, float b, float a) override
		{
			_p->setAmbient(eh::RGBA(r, g, b, a));
		}
		virtual void SetSpecularColor(float r, float g, float b, float a)  override
		{
			_p->setSpecular(eh::RGBA(r, g, b, a));
		}
		virtual void SetSpecularFactor(float f)  override
		{
			_p->setSpecularFactor(f);
		}
		virtual void SetEmissionColor(float r, float g, float b, float a) override
		{
			_p->setEmission(eh::RGBA(r, g, b, a));
		}

		void SetDiffuseTexture(std::wstring fileName) override
		{
			_p->setTexture(eh::Texture::createFromFile(fileName));
		}

		void SetReflectionTexture(std::wstring fileName) override
		{
			_p->setReflTexture(eh::Texture::createFromFile(fileName));
		}

		void SetBumpTexture(std::wstring fileName) override
		{
			_p->setBumpTexture(eh::Texture::createFromFile(fileName));
		}
	};

	struct ShapeNode : public IShapeNode
	{
		eh::Ptr<eh::ShapeNode> _p = nullptr;

		ShapeNode(eh::Ptr<eh::ShapeNode> p) : _p(p)
		{}
	};

	struct GroupNode : public IGroupNode
	{
		eh::Ptr<eh::GroupNode> _p = nullptr;

		GroupNode(eh::Ptr<eh::GroupNode> p) : _p(p)
		{}

		void AddChildNode(std::shared_ptr<ISceneNode> childNode) override
		{
			ISceneNode* p = childNode.get();
			if (auto pp = dynamic_cast<ShapeNode*>(p))
				_p->addChildNodes(pp->_p);
			else if (auto pp = dynamic_cast<GroupNode*>(p))
				_p->addChildNodes(pp->_p);
		}
	};

	std::shared_ptr<IGroupNode> Scene::TryGetGroupNodeFromHandle(void* handle)
	{
		if(auto p = dynamic_cast<eh::GroupNode*>((eh::SceneNode*)handle))
			return std::make_shared<GroupNode>(p);
		//if (auto p = dynamic_cast<eh::ShapeNode*>((eh::SceneNode*)handle))
		//	return std::make_shared<ShapeNode>(p);
		else
			return nullptr;
	}

	void* Scene::NodeToHandle(std::shared_ptr<ISceneNode> node)
	{
		if (auto pp = dynamic_cast<ShapeNode*>(node.get()))
			return pp->_p.get();
		else if (auto pp = dynamic_cast<GroupNode*>(node.get()))
			return pp->_p.get();
		else
			return nullptr;
	}

	std::shared_ptr<IGeometry> Scene::CreateGeometry()
	{
		return std::make_shared<Geometry>(eh::CreateVertexBuffer(sizeof(eh::Vec3) * 2 + sizeof(eh::Float) * 2));
	}

	std::shared_ptr<IMaterial> Scene::CreateMaterial()
	{
		return std::make_shared<Material>(eh::Material::create());
	}

	std::shared_ptr<IShapeNode> Scene::CreateShapeNode(std::shared_ptr<IMaterial> material, std::shared_ptr<IGeometry> geometry)
	{
		auto m = (Material*)material.get();
		auto g = (Geometry*)geometry.get();
		return std::make_shared<ShapeNode>(eh::ShapeNode::create(m->_p, eh::Geometry::create(eh::Geometry::TRIANGLES, g->_p, g->_indices)));
	}

	std::shared_ptr<IGroupNode> Scene::CreateGroupNode(const math3D::Matrix& transform)
	{
		return std::make_shared<GroupNode>(eh::GroupNode::create(eh::SceneNodeVector(), transform));
	}

	void BaseViewPort::SetScene(std::shared_ptr<ISceneNode> sceneNode)
	{
		auto scene = eh::Scene::create();

		ISceneNode* p = sceneNode.get();
		if (auto pp = dynamic_cast<ShapeNode*>(p))
			scene->insertNode(pp->_p);
		else if (auto pp = dynamic_cast<GroupNode*>(p))
			for (auto it : pp->_p->getChildNodes())
				scene->insertNode(pp->_p);

		this->_viewPort->setScene(scene, scene->createOrbitalCamera());
		this->_viewPort->invalidate();
	}

	void SceneIO::RegisterPlugIn(std::shared_ptr<swig::IPlugIn> plugIn)
	{
		struct WrapPlugIn : public eh::SceneIO::IPlugIn
		{
			std::shared_ptr<swig::IPlugIn> _pImpl;

			WrapPlugIn(std::shared_ptr<swig::IPlugIn> plugIn) : _pImpl(plugIn)
			{}

			virtual std::wstring about() const
			{
				return _pImpl->GetAboutString();
			}
			virtual eh::Uint file_type_count() const
			{
				return _pImpl->GetFileTypeCount();
			}
			virtual std::wstring file_type(eh::Uint i) const
			{
				return _pImpl->GetFileType(i);
			}
			virtual std::wstring file_exts(eh::Uint i) const
			{
				return _pImpl->GetFileExtention(i);
			}
			virtual bool canWrite(eh::Uint i) const
			{
				return false;
			}
			virtual bool canRead(eh::Uint i) const
			{
				return true;
			}
			virtual bool read(const std::wstring& aFile, eh::Ptr<eh::Scene> pScene, eh::SceneIO::progress_callback& progress)
			{
				struct myCallBack : public swig::Callback
				{
					myCallBack(eh::SceneIO::progress_callback& progress) : _f(progress)
					{}
					eh::SceneIO::progress_callback& _f;
					void Call(float value) override
					{
						_f(value);
					}
				};

				myCallBack cb(progress);
				auto scene = Scene::CreateGroupNode(math3D::Matrix::Identity());
				if (_pImpl->ReadFile(aFile, Scene::NodeToHandle(scene), &cb))
				{
					ISceneNode* p = scene.get();
					if (auto pp = dynamic_cast<ShapeNode*>(p))
						pScene->insertNode(pp->_p);
					else if (auto pp = dynamic_cast<GroupNode*>(p))
						pScene->insertNode(pp->_p);

					return true;
				}
				else
					return false;
			}

			virtual bool write(const std::wstring& sFile, eh::Ptr<eh::Scene> pScene, eh::SceneIO::progress_callback& progress)
			{
				return false;
			}
		};

				
		eh::SceneIO::getInstance().RegisterPlugIn(std::make_shared<WrapPlugIn>(plugIn));
	}

	std::wstring SceneIO::GetFileWildcards(bool read)
	{
		return eh::SceneIO::getInstance().getFileWildcards(read);
	}

	std::wstring SceneIO::GetAboutString()
	{
		return eh::SceneIO::getInstance().getAboutString();
	}


	std::shared_ptr<IGroupNode> SceneIO::TryRead(std::wstring filePath, Callback* callback/* = nullptr*/)
	{
		auto scene = eh::Scene::create();
		if (eh::SceneIO::getInstance().read(filePath, scene, [callback](float value) { if (callback) callback->Call(value); }))
		{
			return std::make_shared<GroupNode>(eh::GroupNode::create(scene->getNodes(), math3D::Matrix::Identity()));
		}
		else
			return nullptr;
	}

}