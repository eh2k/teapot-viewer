// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "config.h"
#include "RefCounted.h"
#include <set>

namespace eh{

class RenderingVisitor;
class Controller;
class IDriver;
class Scene;
class Camera;

class API_3D Viewport : public RefCounted
{
public:
	enum Flag
	{
		MODE_WIREFRAME		= 0x00000001,
		MODE_ORTHO		= 0x00000002,
		MODE_LIGHTING		= 0x00000004,
		MODE_SHADOW		= 0x00000008,
		MODE_BACKGROUND		= 0x00000010,

		MODE_DRAWAABBTREE	= 0x00000100,
		MODE_DRAWPRIMBOUNDS	= 0x00000200,
		MODE_TRANSPARENS	= 0x00000400,
		MODE_FPS		= 0x00000800,
	};

public:

	Viewport(Ptr<IDriver> pDriver);
	virtual ~Viewport();

	inline void setModeFlag(Flag flag, bool set = true){(set)?m_modeflags|=flag:m_modeflags&=~flag;}
	inline bool getModeFlag(Flag flag) const { return (m_modeflags & flag) != 0;}

	bool isOrthoProjectionEnabled() const;

	void setDisplayRect(int x, int y, int dx, int dy);
	const Rect& getDisplayRect() const;

	void setDriver(Ptr<IDriver> pDriver) { m_pDriver = pDriver; }
	const Ptr<IDriver> getDriver() const { return m_pDriver; }

	void setScene(Ptr<Scene> pScene, Ptr<Camera> pCamera = nullptr);
	Ptr<Scene> getScene() const {return m_pScene;}

	void drawScene();

	bool isValid() const { return m_valid; }
	void invalidate() { m_valid = false; }

	Ray DPtoRay(int x, int y) const;
	Vec3 WPtoDP(const Vec3& world_coord) const;

	Camera& camera() { return *m_pCamera; }

	Controller& control() { return *m_pController; }
	Controller& control() const { return *m_pController; }

private:
	void drawScene(const Matrix& view, const Matrix& proj, bool bSwapBuffer = true);

	RenderingVisitor* m_pRenderingVisitor;
	Ptr<IDriver>	m_pDriver;

	Ptr<Scene>	m_pScene;
	Ptr<Camera>	m_pCamera;

	int		m_modeflags;

	bool		m_valid;

	friend class Controller;
	Ptr<Controller> m_pController;
};

}//end namespace
