#pragma once
#include "config.h"
#include "RefCounted.h"

namespace eh
{
	class SceneNode;
	class Viewport;

	class API_3D Controller: public RefCounted
    {
    public:
        typedef Uint Flags;
        static const Flags LBUTTON = 0x0001;
        static const Flags RBUTTON = 0x0002;
        static const Flags SHIFT   = 0x0004;
        static const Flags CONTROL = 0x0008;
        static const Flags MBUTTON = 0x0010;

        Controller();
        virtual ~Controller();

        virtual void OnMouseMove(Flags nFlags, int x, int y);
        virtual void OnMouseDown(Flags nFlags, int x, int y);
        virtual void OnMouseUp(Flags nFlags, int x, int y);
        virtual void OnMouseWheel(Flags nFlags, short zDelta, int x, int y);
        virtual void OnKeyDown(int nKeyCode);
        virtual void Animate();

        virtual void attachViewport(Viewport& pViewport);
        const Viewport& getViewport() const
        {
            return *m_pViewport;
        }
        Viewport& getViewport()
        {
            return *m_pViewport;
        }

        void reset();

        Matrix getViewMatrix() const;
        Matrix getProjectionMatrix() const;

    protected:
        void zoom(bool in);
        void zoom(Float faktor);

        friend class Viewport;
        Viewport*	m_pViewport;

        Float		m_zoom;
        Matrix		m_Rotation;
        Matrix		m_Rotation2;
        Vec3		m_Translation;

        Ptr<SceneNode>	m_axis;

        Point down;
        Point mouse;
    };

} //end namespace
