#pragma once

#include "IVisitor.h"
#include "SceneNode.h"

namespace eh{

class Scene;

Ptr<SceneNode> doHitTest(const Ray& ray, const Scene& world, Vec3* hitpoint = NULL);

}	//end namespace
