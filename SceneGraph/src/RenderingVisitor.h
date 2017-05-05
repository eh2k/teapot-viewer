#pragma once

#include "ShapeNode.h"
#include "GroupNode.h"

namespace eh{

class Viewport;
class AABBTreeNode;

class RenderingVisitor: public IVisitor
{
	class CullTest;
public:
	RenderingVisitor(Viewport& cam);
	virtual ~RenderingVisitor(){}

	void init(const Matrix& view, const Matrix& proj);

	void drawScene(const AABBTreeNode* aabbtree);
	bool drawNodes(const SceneNodeVector& nodes, bool bDrawAllTransparent = false);

	Uint t;

private:

	virtual void visit(GroupNode& node);
	virtual void visit(ShapeNode& shape);
	virtual void visit(Geometry& node);

	struct BlendedObject
	{
		Ptr<Material>   mat;
		Ptr<Geometry>	geo;
		Matrix		tra;	//Transformation
		Float		dis;	//Distanz zur ViewportPosition (Auge)

		BlendedObject(Ptr<Geometry> g = nullptr, Ptr<Material> m = nullptr, const Matrix& t = Matrix(), Float d = FLT_MAX)
			:mat(m), geo(g), tra(t), dis(d)
		{
		}
		~BlendedObject()
		{
		}

		bool operator < (const BlendedObject& other) const
		{
			return dis > other.dis;		//die entferntesten zuerst
		}
	};

	typedef std::vector< BlendedObject > BlendedObjects;

	Viewport& getViewport() const
	{
		return m_Viewport;
	}

	IDriver& getDriver() const;

	Viewport&		m_Viewport;
	Vec3		m_EyePos;

	BlendedObjects	m_blendedObjects;
	bool			m_bDepthOffsetEnabled;
	Matrix::Stack	m_MatrixStack;
	Frustum			m_frustum;
};

}	//end namespace
