// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "SceneNode.h"
#include "ShapeNode.h"
#include "GroupNode.h"
#include "Camera.h"

namespace eh{

class AABBTreeNode;

class API_3D Scene: public RefCounted
{
public:
	static Ptr<Scene> create();

	virtual ~Scene();

	void addCamera(Ptr<Camera> cam);
	const std::vector< Ptr<Camera> >& getCameras() const;
	Ptr<Camera> createOrbitalCamera() const;

	bool insertNode(Ptr<SceneNode> object);
	bool deleteNode(Ptr<SceneNode> object);
	bool updateNode(Ptr<SceneNode> object);

	const SceneNodeVector& getNodes() const;

	void clear();
	bool isEmpty(){ return (m_objects.size()==0); }

	AABBox getBounding() const;

	const AABBTreeNode* getAABBTree() const;

	bool isAnimated() const;

protected:
	Scene();
private:
	AABBTreeNode* m_pAABBTree;
	void organizeAABBTree();

	std::vector< Ptr<Camera> > m_cameras;
	SceneNodeVector m_objects;
};

}// end namespace
