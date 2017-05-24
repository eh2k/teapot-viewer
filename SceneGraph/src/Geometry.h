// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "RefCounted.h"
#include "VertexBuffer.h"
#include "IVisitor.h"

namespace eh
{
    class API_3D Geometry: public RefCounted
    {
    public:

        enum TYPE
        {
            POINTS         = 0x0000,
            LINES          = 0x0001,
            LINE_STRIP     = 0x0003,
            TRIANGLES      = 0x0004,
            TRIANGLE_STRIP = 0x0005,
            TRIANGLE_FAN   = 0x0006
        };

        virtual ~Geometry();

        static Ptr<Geometry> create(TYPE mode, Ptr<IVertexBuffer> pIVertexBuffer, const Uint_vec& indices = Uint_vec());

        virtual void accept(IVisitor &v)
        {
            v.visit(*this);
        }

        TYPE getType() const
        {
            return m_mode;
        }

        Uint getVertexCount() const
        {
            if (getIndices().size()>0)
                return (Uint)getIndices().size();
            else
                return  (Uint)getVertexBuffer()->getVertexCount();
        }

        const Vec3& getCoord( Uint i ) const
        {
            if (m_indices.size()>0)
	    {
		if(i < m_indices.size())
			return m_pIVertexBuffer->getCoord(m_indices[i]);
		else 
			return Vec3::Null();
	    }
            else
                return m_pIVertexBuffer->getCoord(i);
        }

        Ptr<IVertexBuffer> getVertexBuffer() const
        {
            return m_pIVertexBuffer;
        }
        const Uint_vec&	getIndices() const
        {
            return m_indices;
        }

        virtual const AABBox& getBounding() const
        {
            return m_Bounding;
        }
    private:
        Geometry(TYPE mode, Ptr<IVertexBuffer> pIVertexBuffer, const Uint_vec& indices = Uint_vec());

        TYPE				m_mode;
        Uint_vec			m_indices;
        Ptr<IVertexBuffer>	m_pIVertexBuffer;

        AABBox			m_Bounding;
    public:
        Ptr<IResource>	m_resource;
    };

}//end namespace
