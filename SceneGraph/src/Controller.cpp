
#include "PickingVisitor.h"
#include "RenderingVisitor.h" //Animate
#include "SceneIO.h"
#include "Camera.h"
#include "Viewport.h"
#include "Controller.h"
#include <iostream>

extern "C"
{
	eh::SceneIO::IPlugIn* XcreatePlugIn();
}

namespace eh{

Controller::Controller():
	m_pViewport( nullptr ),
	m_zoom(1.f),
	m_axis( nullptr )
{
	Ptr<Scene> scene = Scene::create();
	std::auto_ptr<SceneIO::IPlugIn> objloader( XcreatePlugIn() );

	struct dummy {static void callback(float f){} };
	SceneIO::progress_callback dummy_cb(dummy::callback);

	objloader->read( L"./media/axis.obj", scene, dummy_cb);

	m_axis = GroupNode::create( scene->getNodes(), Matrix::Scale( Vec3(0.2f, 0.2f, 0.2f))) ;
}
Controller::~Controller()
{
}
void Controller::attachViewport(Viewport& pViewport)
{
	m_pViewport = &pViewport;
}
void Controller::OnMouseMove(Flags nFlags, int x, int y)
{
	Point point(x,y);

	if (nFlags == LBUTTON)
	{
		Vec3 r;

		r.x = -(Float)(point.x - mouse.x)/3;
		r.y = (Float)(point.y - mouse.y)/3;

		r = cross(r, Vec3(0,0,1));

		Matrix rot = Matrix::Rotation( r.normalized(), r.getLen());
		m_Rotation = m_Rotation * rot;
	}

	if (nFlags == (LBUTTON|RBUTTON)  )
	{
		Vec3 r;

		r.x = -(Float)(point.x - mouse.x)/3;
		r.y = (Float)(point.y - mouse.y)/3;

		r = cross(r, Vec3(0,0,1));

		Matrix rot = Matrix::Rotation( r.normalized(), r.getLen());
		m_Rotation2 = m_Rotation2 * rot;
	}

	if (nFlags == RBUTTON )
	{
		Vec3 n = transform(Vec3(0,0,-1), m_Rotation.getInverted() );
		n = n.normalized();

		Plane ebene = Plane(n.x, n.y, n.z, 0);

		Ray a = getViewport().DPtoRay(mouse.x, mouse.y);
		Ray b = getViewport().DPtoRay(point.x, point.y);

		Float ta = 0, tb = 0;
		ebene.getIntersectionWithLine(a.getOrigin(), a.getPointAt(1), ta);
		ebene.getIntersectionWithLine(b.getOrigin(), b.getPointAt(1), tb);

		Vec3 d = a.getPointAt(ta) - b.getPointAt(tb);

		m_Translation = m_Translation + transform(d, m_Rotation );
	}

	mouse = point;
}
void Controller::OnMouseDown(Flags nFlags, int x, int y)
{
	down = mouse = Point(x,y);

	Ptr<SceneNode> pSelected = doHitTest( getViewport().DPtoRay(x, y), *getViewport().getScene(), NULL );

	if(pSelected)
	{
		if(pSelected->Flags() & SceneNode::FLAG_SELECTED)
			pSelected->Flags() &= ~SceneNode::FLAG_SELECTED;
		else
			pSelected->Flags() |= SceneNode::FLAG_SELECTED;
	}
}
void Controller::OnMouseUp(Flags nFlags, int x, int y)
{
	mouse = Point(x,y);
}

void Controller::OnMouseWheel(Flags nFlags, short zDelta, int x, int y)
{
	if(zDelta<0)
		this->zoom(true);
	else
		this->zoom(false);
}

void Controller::Animate()
{
	getViewport().m_pRenderingVisitor->t++;
	getViewport().invalidate();
}

void Controller::OnKeyDown(int keycode)
{
	Vec3 center = getViewport().getScene()->getBounding().getCenter();
	Float f = (getViewport().m_pCamera->m_pos - center).getLen()/100;
	Matrix r = m_Rotation2 *  Matrix::Scale(Vec3(f,f,f));

	switch(keycode)
	{
	case 38:
	case 315: //forward
		this->m_Translation += transform(Vec3(0,0,-1),r);
		break;
	case 40:
	case 317: //back
		this->m_Translation -= transform(Vec3(0,0,-1),r);
		break;
	case 37:
	case 314: //left
		this->m_Translation -= transform(Vec3(1,0,0),r);
		break;
	case 39:
	case 316: //right
		this->m_Translation += transform(Vec3(1,0,0),r);
		break;
	case 33:
	case 366: //up
		this->m_Translation += transform(Vec3(0,1,0),r);
		break;
	case 34:
	case 367: //down
		this->m_Translation -= transform(Vec3(0,1,0),r);
		break;
	}
}

void Controller::zoom(Float faktor)
{
	if(faktor <= 0.0f)
		return;

	m_zoom *= faktor;
}

void Controller::zoom(bool in)
{
	zoom((in)?1.2f:0.8f);
}

void Controller::reset()
{
	m_zoom = 1.f;
	m_Rotation = getViewport().m_pCamera->getRotation();
	m_Rotation2 = Matrix::Identity();

	Vec3 center = getViewport().getScene()->getBounding().getCenter();
	m_Translation = transform( getViewport().m_pCamera->getPosition() - center, m_Rotation) + center;
}

Matrix Controller::getProjectionMatrix() const
{
	Ptr<Camera> c = getViewport().m_pCamera;

	Float w = c->m_width;
	Float h = c->m_height;
	Float n = c->m_near;
	Float f = c->m_far;

	if( getViewport().isOrthoProjectionEnabled() )
	{
		///

		Vec3 pos = c->getPosition();
		Vec3 dir = c->getDirection();
		Vec3 up  = c->getUpVector();

		Vec3 center = getViewport().getScene()->getBounding().getCenter();

		Matrix r = m_Rotation.getInverted();
		Matrix k = c->getRotation();

		pos = transform( pos - center, k*r) + center - m_Translation;

		///

		Float ff = n / ( (pos-center).getLen() );

		w /= ff;
		h /= ff;
	}

	w *= m_zoom;
	h *= m_zoom;

	Float waspect = w/h;
	Float daspect = getViewport().getDisplayRect().Width()/getViewport().getDisplayRect().Height();

	if( daspect < waspect )
		h = w/daspect;
	else
		w = h*daspect;

	h /= 2;
	w /= 2;

	if(getViewport().isOrthoProjectionEnabled())
		return Matrix::Ortho(-w, +w, -h, +h, n, f);
	else
		return Matrix::Frustum(-w, +w, -h, +h, n, f);
}

Matrix Controller::getViewMatrix() const
{
	Ptr<Camera> c =  getViewport().m_pCamera;

	Vec3 center = getViewport().getScene()->getBounding().getCenter();

	Matrix r = m_Rotation.getInverted();
	Matrix k = c->getRotation();

	Vec3 pos = transform( m_Translation - center, r) + center;
	Vec3 dir = transform( Vec3(0,0,-1), m_Rotation2 * r);
	Vec3 up  = transform( Vec3(0,1,0), m_Rotation2 * r);

	return Matrix::Lookat(pos, dir, up );
}

} //end namespace
