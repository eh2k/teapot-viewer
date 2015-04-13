#include "ShapeNode.h"

using namespace eh;

Ptr<ShapeNode> ShapeNode::create()
{
	return new ShapeNode();
}
////////

ShapeNode::ShapeNode()
{}

ShapeNode::~ShapeNode()
{}

void ShapeNode::addGeometry(Ptr<Material> pMat, Ptr<Geometry> pGeo)
{
	if(pGeo != NULL)
	{
		if(pMat == NULL)
			pMat = Material::White();

		Ptr<Geometry>& g = m_geometry[ std::make_pair(pMat, pGeo->getType()) ];

		if(g == NULL)
			g = pGeo;
		else
		{
			assert(0);
		}

		calcBounding();
	}
}

const AABBox& ShapeNode::calcBounding()
{
	m_BoundingBox = AABBox(Vec3(0,0,0), Vec3(0,0,0));

	for(GeometryIterator git = GeometryBegin(); git != GeometryEnd(); git++ )
	{
		if(!m_BoundingBox.valid())
			m_BoundingBox = git.getGeometry()->getBounding();
		else
			m_BoundingBox = m_BoundingBox + git.getGeometry()->getBounding();
	}

	return m_BoundingBox;
}
