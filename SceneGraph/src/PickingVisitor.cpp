#include "ShapeNode.h"
#include "GroupNode.h"
#include "AABBTree.h"
#include "Scene.h"
#include "PickingVisitor.h"
#include <map>

namespace eh{

class PickingVisitor: public IVisitor
{
	Ray m_ray;
	const Scene* m_pScene;
public:
	SceneNode* hit;
	std::map<Float, SceneNode*> m_hits;
	Float getMinT() const;

	PickingVisitor(const Ray& ray, const Scene* pScene = NULL);
	void doHitTest(const AABBTreeNode* aabbtree);

	virtual void visit(Geometry& node);
	virtual void visit(ShapeNode& node);
	virtual void visit(GroupNode& node);
};

PickingVisitor::PickingVisitor(const Ray& ray, const Scene* pScene/* = NULL*/):
	m_ray(ray), m_pScene(pScene)
{
}

Float PickingVisitor::getMinT() const
{
	if(m_hits.size()>0)
		return m_hits.begin()->first;
	else
		return FLT_MAX;
}

void PickingVisitor::doHitTest(const AABBTreeNode* aabbtree)
{
	if( aabbtree )
	{
		Float t = 0;
		if(m_ray.getIntersectionWithAABBox(*aabbtree, t))
		{
			for (SceneNodeList::const_iterator it = aabbtree->nodes().begin() ;
			     it != aabbtree->nodes().end( ) ; ++it )
			{
				(*it)->accept(*this);
			}

			doHitTest(aabbtree->left());
			doHitTest(aabbtree->right());
		}
	}
}



void PickingVisitor::visit(Geometry& node)
{
	Float t = 0;
	if(m_ray.getIntersectionWithAABBox(node.getBounding(), t))
	{
		switch(node.getType())
		{
		case Geometry::POINTS:
			break;
		case Geometry::LINES:
			break;
		case Geometry::LINE_STRIP:
			break;
		case Geometry::TRIANGLES:

			for(Uint i = 0, n = node.getVertexCount(); i < n; i+=3)
				if(m_ray.getIntersectionWithTriangle(node.getCoord(i+2), node.getCoord(i+1), node.getCoord(i), t) && t < getMinT())
					m_hits.insert(std::make_pair(t, hit));

			break;
		case Geometry::TRIANGLE_STRIP:
			for(Uint i = 0, n = node.getVertexCount()-2; i < n; i++)
			{
				if(i%2==0)
				{
					if(m_ray.getIntersectionWithTriangle(node.getCoord(i+2), node.getCoord(i+1), node.getCoord(i), t) && t < getMinT())
						m_hits.insert(std::make_pair(t, hit));
				}
				else
				{
					if(m_ray.getIntersectionWithTriangle(node.getCoord(i), node.getCoord(i+1), node.getCoord(i+2), t) && t < getMinT())
						m_hits.insert(std::make_pair(t, hit));
				}
			}
			break;
		case Geometry::TRIANGLE_FAN:
			for(Uint i = 0, n = node.getVertexCount()-1; i < n; i++)
			{
				if(m_ray.getIntersectionWithTriangle(node.getCoord(i+1), node.getCoord(i), node.getCoord(0), t) && t < getMinT())
					m_hits.insert(std::make_pair(t, hit));
			}
			break;
		default:
			assert(0); break;
		}

	}
}

void PickingVisitor::visit(GroupNode& node)
{
	Float t = 0;
	if(m_ray.getIntersectionWithAABBox(node.getBounding(), t))
	{
		Ray bak = m_ray;

		m_ray = transform(m_ray, node.getTransform().getInverted());

		const SceneNodeVector& nodes =  node.getChildNodes();
		for(SceneNodeVector::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
			(*it)->accept(*this);

		m_ray = bak;
	}
}

void PickingVisitor::visit(ShapeNode& shape)
{
	hit = &shape;

	Float t = 0;
	if( m_ray.getIntersectionWithAABBox(shape.getBounding(), t))
	{
		for(GeometryIterator it = shape.GeometryBegin(); it != shape.GeometryEnd(); ++it)
			it.getGeometry()->accept(*this);
	}
}

Ptr<SceneNode> doHitTest(const Ray& ray, const Scene& world, Vec3* hitpoint)
{
	PickingVisitor v(ray, &world);
	v.doHitTest(world.getAABBTree());

	if(v.m_hits.size()>0)
	{
		if(hitpoint)
			*hitpoint = ray.getPointAt(v.m_hits.begin()->first);

		return v.m_hits.begin()->second;
	}
	else
		return NULL;
}

} //end namespace
