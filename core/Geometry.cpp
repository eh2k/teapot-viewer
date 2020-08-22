// Copyright (c) 2007,2010, Eduard Heidt

#include "Geometry.h"
#include <algorithm>

namespace eh{

//////////////////////////////////////////////////////////////////////////

Ptr<Geometry> Geometry::create(Geometry::TYPE mode, Ptr<IVertexBuffer> pIVertexBuffer, const Uint_vec& indices /*= Uint_vec()*/)
{
	if(pIVertexBuffer != nullptr)
		return new Geometry(mode, pIVertexBuffer, indices);
	else
		return nullptr;
}

Geometry::Geometry(Geometry::TYPE mode, Ptr<IVertexBuffer> pIVertexBuffer, const Uint_vec& indices):
	m_mode(mode),
	m_indices(indices),
	m_pIVertexBuffer(pIVertexBuffer)
{
	bool first = true;
	Vec3 b_min, b_max;
	for(Uint i = 0; i < getVertexCount(); i++)
	{
		const Vec3& v = getCoord(i);;

		if(first)
		{
			b_min = v;
			b_max = v;
			first = false;
		}
		else
		{
			b_min.x = math3D::fmin(v.x, b_min.x);
			b_min.y = math3D::fmin(v.y, b_min.y);
			b_min.z = math3D::fmin(v.z, b_min.z);

			b_max.x = math3D::fmax(v.x, b_max.x);
			b_max.y = math3D::fmax(v.y, b_max.y);
			b_max.z = math3D::fmax(v.z, b_max.z);
		}
	}

	m_Bounding = AABBox(b_min, b_max);
}

Geometry::~Geometry()
{}

}//end namespace eh
