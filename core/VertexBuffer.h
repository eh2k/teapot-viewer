// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "config.h"
#include "IDriver.h"

namespace eh
{
    class IVertexBuffer: public RefCounted
    {
    public:
        virtual ~IVertexBuffer(){};

        virtual Uint addVertex(const Vec3& v, const Vec3& n = Vec3(), const Vec3& t = Vec3()) = 0;
        virtual Uint pushVertex(const Vec3& v, const Vec3& n = Vec3(), const Vec3& t = Vec3()) = 0;

        virtual const Vec3& getCoord(Uint i) const = 0;
        virtual const Vec3& getNormal(Uint i) const = 0;
        virtual const Vec3& getTexCoord(Uint i) const = 0;
        virtual Uint getVertexCount() const = 0;

        virtual  const void* getBuffer(Uint offset = 0) const = 0;
        virtual  Uint getStride() const = 0;
        virtual  Uint getBufferSize() const = 0;

        Ptr<IResource> m_resource;
    };

    API_3D Ptr<IVertexBuffer> CreateVertexBuffer(Uint nStride, const void* pBuffer = NULL, Uint nCount = 0);
}
