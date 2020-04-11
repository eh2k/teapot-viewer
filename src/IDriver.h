// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "math3d.hpp"
#include "RefCounted.h"

namespace eh
{

    using namespace math3D;

    class Geometry;
    class Material;

    class IResource: public RefCounted
    {
    protected:
        IResource(){};
    public:
        virtual ~IResource(){};
    };

    class IDriver: public RefCounted
    {
    public:
        IDriver(){}
        virtual ~IDriver(){}	//wichtig wegen MemoryLeaks

        virtual std::string getDriverInformation() const = 0;

        virtual bool beginScene( bool bDrawBG ) = 0;
        virtual bool endScene( bool bShowFPS ) = 0;

        virtual bool drawPrimitive(Geometry& primitive) = 0;
        virtual void draw2DText(const char* text, int x, int y) = 0;
        virtual void setMaterial(const Material* pMaterial) = 0;

        virtual void setViewport(int x, int y, int dx, int dy) = 0;
        virtual const Rect& getViewport() const = 0;

        virtual void setProjectionMatrix(const Matrix& mat) = 0;
        virtual void setViewMatrix(const Matrix& mat) = 0;
        virtual void setWorldMatrix(const Matrix& m) = 0;
        virtual void setShadowMatrix(const Matrix& m) = 0;

        virtual void enableShadow(bool bEnable) = 0;
        virtual void enableWireframe(bool bEnable) = 0;
        virtual void enableLighting(bool bEnable) = 0;
        virtual void enableBlending(bool bEnable) = 0;
        virtual void enableZWriting(bool bEnable) = 0;
        virtual void enableCulling(bool bEnable) = 0;

        virtual void cullFace(bool bEnable) = 0;
        virtual void enableDepthTest(bool enable) = 0;
        virtual void setDepthOffset(Uint n, Float f) = 0;
    };
}//end namespace
