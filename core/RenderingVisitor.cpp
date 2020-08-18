#include "RenderingVisitor.h"
#include "AABBTree.h"
#include "Viewport.h"
#include "IDriver.h"
#include "Camera.h"
#include "Scene.h"
#include "Controller.h"
#include <algorithm>
#include "stdio.h"


using namespace eh;

static Ptr<Geometry> createWireBoxGeometry(const Vec3& center, const Vec3& size)
{
	/*
	 1-----2
	 |\    |\
	 | 5-----6
	 0-|---3 |
	  \|    \|
	   4-----7
	*/

	Vec3 x(size.x/2,0,0);
	Vec3 y(0,size.y/2,0);
	Vec3 z(0,0,size.z/2);

	std::vector<Vec3> vertices;
	vertices.reserve(8);
	vertices.push_back(center - x - y - z);
	vertices.push_back(center - x - y + z);
	vertices.push_back(center + x - y + z);
	vertices.push_back(center + x - y - z);
	vertices.push_back(center - x + y - z);
	vertices.push_back(center - x + y + z);
	vertices.push_back(center + x + y + z);
	vertices.push_back(center + x + y - z);

	Uint_vec indices;

	Ptr<IVertexBuffer> pVB = CreateVertexBuffer( sizeof(Vec3)*2 );

	indices.push_back(pVB->addVertex(vertices[0])); indices.push_back(pVB->addVertex(vertices[1]));
	indices.push_back(pVB->addVertex(vertices[1])); indices.push_back(pVB->addVertex(vertices[2]));
	indices.push_back(pVB->addVertex(vertices[2])); indices.push_back(pVB->addVertex(vertices[3]));
	indices.push_back(pVB->addVertex(vertices[3])); indices.push_back(pVB->addVertex(vertices[0]));
	indices.push_back(pVB->addVertex(vertices[4])); indices.push_back(pVB->addVertex(vertices[5]));
	indices.push_back(pVB->addVertex(vertices[5])); indices.push_back(pVB->addVertex(vertices[6]));
	indices.push_back(pVB->addVertex(vertices[6])); indices.push_back(pVB->addVertex(vertices[7]));
	indices.push_back(pVB->addVertex(vertices[7])); indices.push_back(pVB->addVertex(vertices[4]));
	indices.push_back(pVB->addVertex(vertices[0])); indices.push_back(pVB->addVertex(vertices[4]));
	indices.push_back(pVB->addVertex(vertices[1])); indices.push_back(pVB->addVertex(vertices[5]));
	indices.push_back(pVB->addVertex(vertices[2])); indices.push_back(pVB->addVertex(vertices[6]));
	indices.push_back(pVB->addVertex(vertices[3])); indices.push_back(pVB->addVertex(vertices[7]));

	return Geometry::create(Geometry::LINES, pVB, indices);
}

class RenderingVisitor::CullTest
{
	RenderingVisitor* m_pVisitor;
	static std::vector<unsigned> m_CullStack;
public:

	CullTest(RenderingVisitor* pVisitor, const AABBox& Bounding):
		m_pVisitor(pVisitor)
	{
		m_CullStack.push_back(m_CullStack.back());

		if( m_CullStack.back() == AABBox::INTERSECT )
		{
			if(pVisitor->m_MatrixStack.getStackSize() == 1 )
				m_CullStack.back() = m_pVisitor->m_frustum.isAABBInside(Bounding);
			//else
			//{
			//	const Matrix& world = pVisitor->m_MatrixStack.getTop() * pVisitor->m_SceneMatrixInverted;
			//	const AABBox& bound = Bounding.transformed(world);
			//	m_CullStack.back() = m_pVisitor->m_frustum.IsAABBInFrustum(bound);
			//}
		}
	}

	~CullTest()
	{
		m_CullStack.pop_back();
	}

	unsigned test()
	{
		return m_CullStack.back();
	}
};

std::vector<unsigned> RenderingVisitor::CullTest::m_CullStack(1, AABBox::INTERSECT );

IDriver& RenderingVisitor::getDriver() const
{
	return const_cast<IDriver&>(*m_Viewport.getDriver());
}

RenderingVisitor::RenderingVisitor(Viewport& cam):
		IVisitor(),
		m_Viewport(cam),
		m_bDepthOffsetEnabled(true)
	{
	}

void RenderingVisitor::init(const Matrix& view, const Matrix& projection)
{
	getDriver().setViewMatrix(view);
	getDriver().setProjectionMatrix(projection);
	getDriver().setWorldMatrix(Matrix::Identity());
	m_frustum.extractFrom(projection, view);
	m_MatrixStack.clear(Matrix::Identity());
	m_EyePos = getViewport().camera().getPosition();
}

void RenderingVisitor::drawScene(const AABBTreeNode* aabbtree)
{
	if( aabbtree )
	{
		if(aabbtree->parent() == NULL)
		{
			const AABBox& worldbox = *aabbtree;
			const Vec3& min = worldbox.getMin();

			const Matrix& shadowMatrix = Matrix::Shadow( Plane(0,0,1,-min.z), worldbox.getCenter()+Vec3(0,-worldbox.getSize().y,worldbox.getSize().z*100) );
			getDriver().setShadowMatrix( shadowMatrix );

			m_blendedObjects.resize(0);
		}

		CullTest cull(this, *aabbtree);
		if(cull.test() == AABBox::OUTSIDE)
			return;

		if(getViewport().getModeFlag( Mode::MODE_DRAWAABBTREE ) )
		{
			getDriver().setMaterial( Material::Black().get() );
			createWireBoxGeometry(aabbtree->getCenter(), aabbtree->getSize())->accept(*this);
		}

		for (SceneNodeList::const_iterator it = aabbtree->nodes().begin() ;
		     it != aabbtree->nodes().end( ) ; ++it )
		{
			(*it)->accept(*this);
		}

		drawScene(aabbtree->left());
		drawScene(aabbtree->right());

		if(aabbtree->parent() == NULL)
		{
			// Jetz alles transparente zeichen

			std::sort(m_blendedObjects.begin(), m_blendedObjects.end());

			getDriver().enableBlending(true);
			getDriver().enableZWriting(false);
			int i = 0;

			for(BlendedObjects::const_iterator it = m_blendedObjects.begin(); it != m_blendedObjects.end(); ++it)
			{
				m_MatrixStack.clear(it->tra);
				getDriver().setWorldMatrix(it->tra);
				getDriver().setMaterial(it->mat.get());
				it->geo->accept(*this);

				if(getViewport().getModeFlag(Mode::MODE_TRANSPARENS ))
				{
					getDriver().enableDepthTest(false);
					char s [100]; sprintf(s, "%d", i++);
					Vec3 p = getViewport().WPtoDP( transform(it->geo->getBounding().getCenter(), it->tra) );
					getDriver().draw2DText(s, (int)p.x, (int)p.y);
					getDriver().enableDepthTest(true);
				}

			}
			getDriver().enableZWriting(true);
			getDriver().enableBlending(false);
		}
	}
}
bool RenderingVisitor::drawNodes(const SceneNodeVector& nodes, bool bDrawAllTransparent /*= false*/)
{
	if(nodes.size()==0)
		return false;

	assert( bDrawAllTransparent == false );

	m_blendedObjects.resize(0);

	for (SceneNodeVector::const_iterator it = nodes.begin(); it != nodes.end() ; ++it)
		(*it)->accept(*this);

	// Jetz alles transparente zeichen

	std::sort(m_blendedObjects.begin(), m_blendedObjects.end());

	getDriver().enableBlending(true);
	getDriver().enableZWriting(false);

	for(BlendedObjects::iterator it = m_blendedObjects.begin(); it != m_blendedObjects.end(); ++it)
	{
		m_MatrixStack.clear(it->tra);
		getDriver().setWorldMatrix(it->tra);
		getDriver().setMaterial(it->mat.get());
		it->geo->accept(*this);

	}
	getDriver().enableZWriting(true);
	getDriver().enableBlending(false);

	return true;
}

void RenderingVisitor::visit(Geometry& node)
{
	//CullTest cull(this, node.getBounding());
	//if( cull.test() == AABBox::OUTSIDE )
	//	return false;

	if(m_bDepthOffsetEnabled)
	{
		if( node.getType() == Geometry::LINES)
			getDriver().setDepthOffset(1, 0.0001f);
		else
			getDriver().setDepthOffset(0, 0.0001f);
	}

	getDriver().drawPrimitive(node);

	if(getViewport().getModeFlag(Mode::MODE_DRAWPRIMBOUNDS ) )
	{
		const AABBox& bound = node.getBounding();
		getDriver().setMaterial( Material::Black().get() );
		getViewport().setModeFlag( Mode::MODE_DRAWPRIMBOUNDS, false );
		createWireBoxGeometry(bound.getCenter(), bound.getSize())->accept(*this);
		getViewport().setModeFlag( Mode::MODE_DRAWPRIMBOUNDS, true );
		getDriver().setMaterial( Material::Black().get() );
	}
}

void RenderingVisitor::visit(GroupNode& node)
{
	CullTest cull(this, node.getBounding());
	if(cull.test() == AABBox::OUTSIDE)
		return;

	m_MatrixStack.push();
	m_MatrixStack.mult(node.getTransform(t));

	getDriver().setWorldMatrix(m_MatrixStack.getTop());

	for(SceneNodeVector::const_iterator it = node.getChildNodes().begin(); it!= node.getChildNodes().end(); ++it)
		(*it)->accept(*this);

	m_MatrixStack.pop();
	getDriver().setWorldMatrix(m_MatrixStack.getTop());
}

void RenderingVisitor::visit(ShapeNode& shape)
{
	CullTest cull(this, shape.getBounding());
	if( cull.test() == AABBox::OUTSIDE )
		return;

	SceneNode::FLAGS flags = shape.getFlags();

	if(flags & SceneNode::FLAG_UNVISIBLE)
		return;

	if((flags & SceneNode::FLAG_SELECTED)>0)
	{
		getDriver().setMaterial( Material::Black().get() );
		createWireBoxGeometry( shape.getBounding().getCenter(), shape.getBounding().getSize());
	}

	m_MatrixStack.push();
	getDriver().setWorldMatrix(m_MatrixStack.getTop());

	for(GeometryIterator it = shape.GeometryBegin(); it != shape.GeometryEnd(); ++it)
	{
		if(it.getMaterial()->getOpacTexture() ||
		   (it.getMaterial()->getDiffuse().a > 0.f && it.getMaterial()->getDiffuse().a < 1.f))
		{
			const Matrix& m = m_MatrixStack.getTop()*getViewport().control().getViewMatrix();
			Float distance = 0;

			distance = distanceSq(m_EyePos, transform(it.getGeometry()->getBounding().getCenter(), m) );

			Ptr<Material> pMat = it.getMaterial();

			if((flags & SceneNode::FLAG_SELECTED)>0)
				pMat = Material::create(RGBA(0,0,1, pMat->getDiffuse().a));
			else if((flags & SceneNode::FLAG_HIGHLIGHTED)>0)
				pMat = Material::create(RGBA(1,0,0, pMat->getDiffuse().a));

			m_blendedObjects.push_back(BlendedObject(it.getGeometry(), pMat, m_MatrixStack.getTop(), distance ));
			continue;
		}

		if((flags & SceneNode::FLAG_SELECTED)>0)
			getDriver().setMaterial( Material::create(RGBA(0,0,1)).get() );
		else if((flags & SceneNode::FLAG_HIGHLIGHTED)>0)
			getDriver().setMaterial( Material::create(RGBA(1,0,0)).get() );
		else
			getDriver().setMaterial( it.getMaterial().get() );

		it.getGeometry()->accept(*this);
	}

	m_MatrixStack.pop();
}
