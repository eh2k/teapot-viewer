// Copyright (c) 2007,2010, Eduard Heidt

#include "Viewport.h"
#include "Scene.h"
#include "Material.h"
#include "ShapeNode.h"
#include "RenderingVisitor.h"
#include "Controller.h"
#include "IDriver.h"
#include "Camera.h"

#include <iostream>

using namespace eh;

Viewport::Viewport(Ptr<IDriver> pDriver):
	m_pRenderingVisitor(nullptr),
	m_pDriver(pDriver),
	m_pScene(nullptr),
	m_pCamera(nullptr),
	m_modeflags(MODE_BACKGROUND|MODE_LIGHTING|MODE_SHADOW|MODE_FPS),
	m_valid(false),
	m_pController(new Controller())
{
	m_pController->attachViewport(*this);

	m_pScene = Scene::create();
	m_pCamera = m_pScene->createOrbitalCamera();

	m_pRenderingVisitor = new RenderingVisitor(*this);
}
Viewport::~Viewport()
{
	if(m_pRenderingVisitor)
		delete m_pRenderingVisitor;
}

void Viewport::setDisplayRect(int x, int y, int dx, int dy)
{
	m_pDriver->setViewport(x, y, dx, dy);
}

const Rect& Viewport::getDisplayRect() const
{
	return m_pDriver->getViewport();
}

void Viewport::setScene(Ptr<Scene> pScene, Ptr<Camera> pCamera )
{
	m_pScene = pScene;
	m_pCamera = pCamera;

	if( m_pCamera == nullptr )
	{
		if( m_pScene->getCameras().size()>0)
			m_pCamera = m_pScene->getCameras()[0];
		else
			m_pCamera = m_pScene->createOrbitalCamera();
	}

	control().reset();
}

void Viewport::drawScene()
{
	if(m_pCamera == nullptr)
		return;

	drawScene( control().getViewMatrix(), control().getProjectionMatrix(), true );
	m_valid = true;
}

void Viewport::drawScene(const Matrix& view, const Matrix& proj, bool bRenderToWindow)
{
	if(m_pScene == nullptr)
		return;

	if(m_pDriver->beginScene(  getModeFlag(Mode::MODE_BACKGROUND) ))
	{
		m_pDriver->enableLighting( getModeFlag(Mode::MODE_LIGHTING) );
		m_pDriver->enableShadow( getModeFlag(Mode::MODE_SHADOW) );

		m_pDriver->enableDepthTest(true);

		m_pDriver->enableWireframe(getModeFlag(Mode::MODE_WIREFRAME));

		m_pRenderingVisitor->init(view, proj);

		//TODO
		//for(size_t i = 0; i < getScene()->getCameras().size(); i++)
		//{
		//	Camera::ptr cam = getScene()->getCameras()[i];
		//}

		m_pRenderingVisitor->drawScene(getScene()->getAABBTree());

		/// AXIS ///

		if(control().m_axis)
		{
			m_pDriver->enableShadow(false);

			Matrix view = control().m_Rotation * Matrix::Translation(Vec3(50.f, 50.f, 50.f));
			Matrix proj = Matrix::Ortho(0, (Float)getDisplayRect().Width(), 0, (Float)getDisplayRect().Height(), -500.f, +500.f);
			m_pRenderingVisitor->init(view, proj);

			m_pRenderingVisitor->drawNodes(control().m_axis);
		}

		m_pDriver->endScene( getModeFlag(Mode::MODE_FPS) );
	}
}

bool Viewport::isOrthoProjectionEnabled() const
{
	return getModeFlag(MODE_ORTHO);
}

Ray Viewport::DPtoRay(int x, int y) const
{
	return Ray(x, y, getDisplayRect(), control().getProjectionMatrix(), control().getViewMatrix());
}

Vec3 Viewport::WPtoDP(const Vec3& world_coord) const
{
	Vec3 ret = math3D::project(world_coord, getDisplayRect(), control().getProjectionMatrix(), control().getViewMatrix());
	ret.y = getDisplayRect().Height() - ret.y;
	return ret;
}
