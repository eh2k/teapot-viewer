// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "SceneNode.h"

namespace eh{

class API_3D GroupNode: public SceneNode
{
private:
	SceneNodeVector m_vNodes;
public:

	void addChildNodes(const SceneNodeVector& nodes);
	void deleteChildNodes();

	const SceneNodeVector& getChildNodes() const {return m_vNodes;}

public:
	static Ptr<GroupNode> create( const SceneNodeVector &nodes = SceneNodeVector(), const Matrix& m = Matrix::Identity() );
	static Ptr<GroupNode> createAnimated( const SceneNodeVector &nodes, const std::vector<Matrix>& transform_sequence );

	virtual ~GroupNode();

	inline void setTransform(const Matrix& m)
	{
		m_matrix[0] = m;
		calcBounding();
	}

	const Matrix& getTransform(Uint t = 0) const
	{
		return m_matrix[t%m_matrix.size()];
	}

	bool isAnimated() const
	{
		return m_matrix.size() > 1;
	}

	virtual void accept(IVisitor &v)
	{
		v.visit(*this);
	}

	virtual const AABBox& calcBounding();
private:
	friend class Viewport;

	GroupNode(const SceneNodeVector& nodes, const std::vector<Matrix>& m );

	std::vector<Matrix> m_matrix;


};

}//end namespace
