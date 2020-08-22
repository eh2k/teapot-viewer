// Copyright (c) 2007,2010, Eduard Heidt

#include "GroupNode.h"
#include <algorithm>

namespace eh
{

Ptr<GroupNode> GroupNode::create(const SceneNodeVector& nodes,  const Matrix& m )
{
	return new GroupNode( nodes, std::vector<Matrix>(1, m) );
}

Ptr<GroupNode>  GroupNode::createAnimated( const SceneNodeVector &nodes, const std::vector<Matrix>& transform_sequence )
{
	return new GroupNode( nodes, transform_sequence );
}

void GroupNode::addChildNodes(const SceneNodeVector& nodes)
{
	if(nodes.size() == 0)
		return;

	m_vNodes.reserve( m_vNodes.capacity() - m_vNodes.size() + nodes.size() );

	for (SceneNodeVector::const_iterator it = nodes.begin() ; it != nodes.end( ) ; it++ )
		if(*it != nullptr && std::find(m_vNodes.begin(), m_vNodes.end(), *it) == m_vNodes.end()) //no dups
			m_vNodes.push_back(*it);

	calcBounding();
}

void GroupNode::deleteChildNodes()
{
	m_vNodes.clear();
	m_BoundingBox = AABBox(Vec3(0,0,0), Vec3(0,0,0));
}

GroupNode::GroupNode(const SceneNodeVector &nodes, const std::vector<Matrix>& tra_seq ):
	m_matrix(tra_seq)
{
	addChildNodes(nodes);
}

GroupNode::~GroupNode()
{}

const AABBox& GroupNode::calcBounding()
{
	m_BoundingBox = AABBox(Vec3(0,0,0), Vec3(0,0,0));

	if(m_vNodes.size()>0)
		m_BoundingBox = (*m_vNodes.begin())->getBounding();

	for (SceneNodeVector::iterator it = m_vNodes.begin() ; it != m_vNodes.end( ) ; it++ )
		m_BoundingBox = m_BoundingBox + (*it)->getBounding();

	m_BoundingBox = transform(m_BoundingBox, getTransform());
	return m_BoundingBox;
}

} //end namespace
