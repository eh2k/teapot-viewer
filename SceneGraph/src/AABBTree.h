/****************************************************************************
* Copyright (C) 2007-2010 by E.Heidt  http://teapot-viewer.sourceforge.net/ *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
****************************************************************************/

#pragma once

#include "SceneNode.h"
#include <unordered_map>
#include <cassert>

namespace eh{

class AABBTreeNode: public AABBox
{
protected:
	static const Uint nSceneNodesPerNode = 7;
	typedef std::unordered_map<SceneNode*, AABBTreeNode*> NodeTreeMap;

	NodeTreeMap& m_NodeTreeMap;
	SceneNodeList m_nodes;

	AABBTreeNode* m_left;
	AABBTreeNode* m_right;
	AABBTreeNode* m_parent;

	void deleteTreeNode()
	{
		if(m_parent == NULL)
			return;

		if(m_parent->m_left == this)
			m_parent->m_left = NULL;

		if(m_parent->m_right == this)
			m_parent->m_right = NULL;

		delete this;
	}
public:

	AABBTreeNode(const Vec3& min, const Vec3& max, const SceneNodeList& nodes,
				 NodeTreeMap& theNodeTreeMap, AABBTreeNode* parent):
		AABBox(min, max),
		m_NodeTreeMap(theNodeTreeMap),
		m_left(NULL),
		m_right(NULL),
		m_parent(parent)
	{
		if(parent)
			divideAABBox(nodes);
	}
	virtual ~AABBTreeNode()
	{
		if(m_left)
			delete m_left;
		if(m_right)
			delete m_right;
	}

	const SceneNodeList& nodes() const { return m_nodes; }
	const AABBTreeNode* parent() const { return m_parent; }
	const AABBTreeNode* left() const { return m_left;}
	const AABBTreeNode* right() const { return m_right;}

	bool divideAABBox(const SceneNodeList& nodes)
	{
		AABBox aa, bb;

		divide(aa, bb);

		SceneNodeList aao;
		SceneNodeList bbo;

		for(SceneNodeList::const_iterator it = nodes.begin(); it!= nodes.end(); it++)
		{
			const Ptr<SceneNode> o = *it;
			const AABBox& box = o->getBounding();

			bool bValid = box.valid();
			bool bInsideA = aa.isInside(box) == AABBox::INSIDE;
			bool bInsideB = bb.isInside(box) == AABBox::INSIDE;

			if( !bValid || ( !bInsideA && !bInsideB) )
			{
				m_nodes.push_back(*it);
				assert(m_NodeTreeMap.find((*it).get()) == m_NodeTreeMap.end());
				m_NodeTreeMap[(*it).get()] = this;
			}
			else if( bInsideA )
				aao.push_back(*it);
			else if( bInsideB )
				bbo.push_back(*it);
			else
				assert(0); //Darf nicht sien
		}


		if(aao.size()>nSceneNodesPerNode)
			m_left = new AABBTreeNode(aa.getMin(), aa.getMax(), aao, m_NodeTreeMap, this);
		else
			for(SceneNodeList::iterator it = aao.begin(); it!= aao.end(); it++)
			{
				m_nodes.push_back(*it);
				assert(m_NodeTreeMap.find((*it).get()) == m_NodeTreeMap.end());
				m_NodeTreeMap[(*it).get()] = this;
			}

		if(bbo.size()>nSceneNodesPerNode)
			m_right = new AABBTreeNode(bb.getMin(), bb.getMax(), bbo, m_NodeTreeMap, this);
		else
			for(SceneNodeList::iterator it = bbo.begin(); it!= bbo.end(); it++)
			{
				m_nodes.push_back(*it);
				assert(m_NodeTreeMap.find((*it).get()) == m_NodeTreeMap.end());
				m_NodeTreeMap[(*it).get()] = this;
			}

		return true;
	}

	bool insertNode(Ptr<SceneNode> node)
	{
		assert(node);

		if(isInside(node->getBounding()) != AABBox::INSIDE)
			return false;

		if((m_right && m_right->insertNode(node)) || (m_left && m_left->insertNode(node)))
			return true;

		m_nodes.push_back(node);

		assert(m_NodeTreeMap.find(node.get()) == m_NodeTreeMap.end());
		m_NodeTreeMap[node.get()] = this;

		if(m_nodes.size()>nSceneNodesPerNode)
		{
			SceneNodeList nodes = m_nodes;

			for(SceneNodeList::iterator it = m_nodes.begin(); it!=m_nodes.end(); it++)
			{
				m_NodeTreeMap.erase((*it).get());
				assert(m_NodeTreeMap.find((*it).get()) == m_NodeTreeMap.end());
			}
			m_nodes.clear();

			divideAABBox(nodes);

		}

		return true;
	}

	void deleteNode(Ptr<SceneNode> node)
	{
		assert(node);

		if(m_NodeTreeMap.find(node.get()) == m_NodeTreeMap.end())
			return;

		if(m_NodeTreeMap[node.get()] != this)
		{
			m_NodeTreeMap[node.get()]->deleteNode(node);
			return;
		}

		m_NodeTreeMap.erase(node.get());
		assert(m_NodeTreeMap.find(node.get()) == m_NodeTreeMap.end());

		if(m_nodes.size()>1 || m_left || m_right)
			m_nodes.erase(std::find(m_nodes.begin(), m_nodes.end(), node));
		else
			deleteTreeNode();
	}
};

class AABBTreeRoot: public AABBTreeNode
{
private:
	NodeTreeMap  m_rootNodeTreeMap;
public:
	AABBTreeRoot(const Vec3& min, const Vec3& max, const SceneNodeList& nodes):
		AABBTreeNode(min, max, nodes, m_rootNodeTreeMap, NULL)
	{
		divideAABBox(nodes);
	}

	virtual ~AABBTreeRoot()
	{
	}
};

//////////////////////////////////////////////////////////////////////////
}//end namespace
