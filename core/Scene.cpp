// Copyright (c) 2007,2010, Eduard Heidt

#include "Scene.h"
#include "AABBTree.h"

namespace eh
{

    Ptr<Scene> Scene::create()
    {
        return new Scene();
    }

    Scene::Scene():
            m_pAABBTree(NULL)
    {
    }
    Scene::~Scene()
    {
        clear();
    }

    bool Scene::insertNode(Ptr<SceneNode> object)
    {
        if (object && std::find(m_objects.begin(), m_objects.end(), object) == m_objects.end())
        {
            m_objects.push_back(object);

            if (m_pAABBTree == NULL)
                organizeAABBTree();
            else if (m_pAABBTree->isInside(object->getBounding()) == AABBox::INSIDE)
                m_pAABBTree->insertNode(object);
            else	// Node outside -> reorganize
                organizeAABBTree();

            return true;
        }

        return false;
    }

    bool Scene::deleteNode(Ptr<SceneNode> object)
    {
        SceneNodeVector::iterator it = std::find(m_objects.begin(), m_objects.end(), object);
        if (it != m_objects.end())
        {
            if (m_pAABBTree)
                m_pAABBTree->deleteNode(object);

            m_objects.erase(it);

            return true;
        }
        else
            return false;
    }

    bool Scene::updateNode(Ptr<SceneNode> object)
    {
        if (m_pAABBTree)
        {
            if (m_pAABBTree->isInside(object->getBounding()) == AABBox::INSIDE)
            {
                m_pAABBTree->deleteNode(object);
                m_pAABBTree->insertNode(object);
            }
            else
                organizeAABBTree();
        }

        return true;
    }

    void Scene::organizeAABBTree()
    {
        if (m_pAABBTree)
            delete m_pAABBTree;

        const AABBox& bound = getBounding();

        SceneNodeList v;

        for (auto it = m_objects.cbegin(), end = m_objects.cend( ) ; it!=end; ++it )
            v.push_back(*it);

        m_pAABBTree = new AABBTreeRoot(bound.getMin(), bound.getMax(), v);
    }

    void Scene::clear()
    {
        if (m_pAABBTree)
            delete m_pAABBTree;

        m_pAABBTree = NULL;

        m_objects.clear();
        m_cameras.clear();
    }

    AABBox Scene::getBounding() const
    {
        Vec3 b_min, b_max;

        bool	first = true;
        for (SceneNodeVector::const_iterator it = m_objects.begin(), end = m_objects.end( ) ; it!=end; ++it )
        {
            if ( (*it)->getFlags() & SceneNode::FLAG_UNVISIBLE)
                continue;

            if ( (*it)->getBounding().valid() == false)
                continue;

            if (first)
            {
                b_min = (*it)->getBounding().getMin();
                b_max = (*it)->getBounding().getMax();
                first = false;
            }
            else
            {
                const Vec3& bmin = (*it)->getBounding().getMin();
                const Vec3& bmax = (*it)->getBounding().getMax();

                b_min.x = math3D::fmin(b_min.x, bmin.x);
                b_min.y = math3D::fmin(b_min.y, bmin.y);
                b_min.z = math3D::fmin(b_min.z, bmin.z);

                b_max.x = math3D::fmax(b_max.x, bmax.x);
                b_max.y = math3D::fmax(b_max.y, bmax.y);
                b_max.z = math3D::fmax(b_max.z, bmax.z);
            }
        }

        return AABBox(b_min, b_max);
    }

    const AABBTreeNode* Scene::getAABBTree() const
    {
        return m_pAABBTree;
    }

    const SceneNodeVector& Scene::getNodes() const
    {
        return m_objects;
    }

    bool Scene::isAnimated() const
    {
        struct animated
        {
            bool test(const SceneNodeVector& nodes)
            {
                for (auto node : nodes)
                {
                    if (auto pGroup = dynamic_cast<GroupNode*>(node.get()))
                    {
                        if (pGroup->isAnimated())
                            return true;

                        if ( test( pGroup->getChildNodes()) )
                            return true;
                    }
                }

                return false;
            }
        };

        return animated().test(getNodes());
    }

    Ptr<Camera> Scene::createOrbitalCamera() const
    {
        const AABBox& box = getBounding();
        Float longest = math3D::fmax(math3D::fmax(box.getSize().x, box.getSize().y), box.getSize().z);
        Vec3 d = Vec3(longest/2,longest/2,longest/2) * 1.1f;

        Vec3 min = box.getCenter()-d;
        Vec3 max = box.getCenter()+d;
        Vec3 size = max - min;

        Float near = size.getLen();
        Float far =  near*3;

        return Camera::create("Default Orbital Camera", size.x, size.y, near, far,
                              box.getCenter()+Vec3(0,0,(far/*-near*/)/2), Vec3(0,0,-1), Vec3(0,1,0) );
    }

    void Scene::addCamera(Ptr<Camera> cam)
    {
        m_cameras.push_back(cam);
    }

    const std::vector< Ptr<Camera> >& Scene::getCameras() const
    {
        return m_cameras;
    }

}	//end namespace
