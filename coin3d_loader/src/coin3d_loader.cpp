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

#include <SceneIO.h>
using namespace eh;

#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShape.h>

#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/fields/SoFieldData.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>

#include <Inventor/VRMLnodes/SoVRMLShape.h>
#include <Inventor/VRMLnodes/SoVRMLGeometry.h>
#include <Inventor/VRMLnodes/SoVRMLTransform.h>
#include <Inventor/VRMLnodes/SoVRMLIndexedFaceSet.h>
#include <Inventor/VRMLnodes/SoVRMLPositionInterpolator.h>
#include <Inventor/VRMLnodes/SoVRMLOrientationInterpolator.h>

#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoCallbackAction.h>

#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/nodes/SoLight.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoSpotLight.h>
#include <Inventor/nodes/SoPointLight.h>
#include <Inventor/nodes/SoRotor.h>
#include <Inventor/nodes/SoPendulum.h>
#include <Inventor/nodes/SoShuttle.h>
#include <Inventor/nodes/SoLOD.h>
#include <Inventor/misc/SoChildList.h>

#include <iostream>
#include <string.h>
#include <math.h>
#include <map>

static_assert(sizeof(SbMatrix) == sizeof(Matrix));

class Coin3DLoader: public SceneIO::IPlugIn
{
    struct GeoetryStruct
    {
        Ptr<IVertexBuffer> m_pVB;
        std::map<int, Ptr<Material> > m_mapMaterials;
        std::map<int, Uint_vec> m_mapMaterialTriangles;
        std::map<int, Uint_vec> m_mapMaterialLines;
        std::map<int, Uint_vec> m_mapMaterialPoints;
    };

public:

    Coin3DLoader()
    {
    }
    virtual ~Coin3DLoader()
    {

    }

    static void addTriangleCB(void* data, SoCallbackAction* action, const SoPrimitiveVertex *v0, const SoPrimitiveVertex *v1, const SoPrimitiveVertex *v2)
    {
        GeoetryStruct* _this = (GeoetryStruct*)(data);
        int material_index = v0->getMaterialIndex();

        if (_this->m_mapMaterials[material_index] == NULL)
        {
            SbColor ambient, diffuse, specular, emission;
            float transparency, shininess;
            action->getMaterial(ambient, diffuse, specular, emission, shininess, transparency, material_index);

            Ptr<Material> m = Material::create( RGBA(diffuse[0], diffuse[1], diffuse[2], transparency) );
            m->setAmbient( RGBA(ambient[0], ambient[1], ambient[2]) );
            m->setSpecular( RGBA(specular[0], specular[1], specular[2]) );
            m->setSpecularFactor( shininess );
            m->setEmission( RGBA(emission[0], emission[1], emission[2]) );
            _this->m_mapMaterials[material_index] = m;
        }

        _this->m_mapMaterialTriangles[material_index].push_back( _this->m_pVB->addVertex( (Vec3&)v0->getPoint(), (Vec3&)v0->getNormal()) );
        _this->m_mapMaterialTriangles[material_index].push_back( _this->m_pVB->addVertex( (Vec3&)v1->getPoint(), (Vec3&)v1->getNormal()) );
        _this->m_mapMaterialTriangles[material_index].push_back( _this->m_pVB->addVertex( (Vec3&)v2->getPoint(), (Vec3&)v2->getNormal()) );

    }
    static void addLineSegmentCB(void* data, SoCallbackAction* action, const SoPrimitiveVertex *v0, const SoPrimitiveVertex *v1)
    {
        GeoetryStruct* _this = (GeoetryStruct*)(data);
        int material_index = v0->getMaterialIndex();

        _this->m_mapMaterialLines[material_index].push_back( _this->m_pVB->addVertex( (Vec3&)v0->getPoint(), (Vec3&)v0->getNormal()) );
        _this->m_mapMaterialLines[material_index].push_back( _this->m_pVB->addVertex( (Vec3&)v1->getPoint(), (Vec3&)v1->getNormal()) );
    }
    static void addPointCB(void* data, SoCallbackAction* action,  const SoPrimitiveVertex *v0)
    {
        GeoetryStruct* _this = (GeoetryStruct*)(data);
        int material_index = v0->getMaterialIndex();

        _this->m_mapMaterialPoints[material_index].push_back( _this->m_pVB->addVertex( (Vec3&)v0->getPoint(), (Vec3&)v0->getNormal()) );
    }

    IVertexBuffer* m_pVB;
    std::map<uint32_t, Ptr<SceneNode> >* m_pMapNodes;

    Ptr<SceneNode> makeShape(SoNode* node)
    {
        std::map<uint32_t, Ptr<SceneNode> >::iterator it = m_pMapNodes->find(node->getNodeId());
        if (it != m_pMapNodes->end())
            return it->second;

        SoCallbackAction cbAction;

        GeoetryStruct tmp;
        tmp.m_pVB = m_pVB;

        cbAction.addTriangleCallback(SoVRMLGeometry::getClassTypeId(), addTriangleCB, &tmp);
        cbAction.addLineSegmentCallback(SoVRMLGeometry::getClassTypeId(), addLineSegmentCB, &tmp);
        cbAction.addPointCallback(SoVRMLGeometry::getClassTypeId(), addPointCB, &tmp);

        cbAction.apply(node);

        Ptr<ShapeNode> s = ShapeNode::create();
        (*m_pMapNodes)[node->getNodeId()] = s;

        for (std::map<int, Uint_vec>::const_iterator it = tmp.m_mapMaterialTriangles.begin();  it != tmp.m_mapMaterialTriangles.end(); ++it)
            s->addGeometry( tmp.m_mapMaterials[it->first], Geometry::create(Geometry::TRIANGLES, tmp.m_pVB, it->second) );

        for (std::map<int, Uint_vec>::const_iterator it = tmp.m_mapMaterialLines.begin();  it != tmp.m_mapMaterialLines.end(); ++it)
            s->addGeometry( tmp.m_mapMaterials[it->first], Geometry::create(Geometry::LINES, tmp.m_pVB, it->second) );

        for (std::map<int, Uint_vec>::const_iterator it = tmp.m_mapMaterialPoints.begin();  it != tmp.m_mapMaterialPoints.end(); ++it)
            s->addGeometry( tmp.m_mapMaterials[it->first], Geometry::create(Geometry::POINTS, tmp.m_pVB, it->second) );


        if (s->GeometryBegin() == s->GeometryEnd())
            std::cout << "makeShape, no triangulation with: " << typeid(*node).name() << std::endl;

        return s;

    }

    Ptr<SceneNode> makeShape(SoCoordinate3* pCoordNode, SoMaterial* pMaterials, SoIndexedFaceSet* pIFSNode)
    {
        if (!pCoordNode || !pIFSNode )
            return nullptr;

        std::map<uint32_t, Ptr<SceneNode> >::iterator it = m_pMapNodes->find(pIFSNode->getNodeId());
        if (it != m_pMapNodes->end())
            return it->second;

        Ptr<ShapeNode> s = ShapeNode::create();
        (*m_pMapNodes)[pIFSNode->getNodeId()] = s;

        std::map<int32_t, Uint_vec> indices;
        Ptr<IVertexBuffer> pVB = m_pVB;

        const int32_t* idx = pIFSNode->coordIndex.getValues(0);
        const int32_t* midx = pIFSNode->materialIndex.getValues(0);
        const SbVec3f* coords = pCoordNode->point.getValues(0);
        for (int i = 0, m = 0; i < pIFSNode->coordIndex.getNum();)
        {
            const Vec3 p1((Vec3&)coords[idx[i]]); i++;
            const Vec3 p2((Vec3&)coords[idx[i]]); i++;
            const Vec3 p3((Vec3&)coords[idx[i]]); i++;
            const Vec3& n = math3D::calcNormal(p1, p2, p3);

            int32_t mid = 0;

            if ( pIFSNode->materialIndex.getNum() == pIFSNode->coordIndex.getNum() / 4 )
                mid = midx[m++];
            else if ( pIFSNode->materialIndex.getNum() == pIFSNode->coordIndex.getNum() )
                mid = midx[i];

            indices[mid].push_back( pVB->addVertex(p1, n) );
            indices[mid].push_back( pVB->addVertex(p2, n) );
            indices[mid].push_back( pVB->addVertex(p3, n) );

            while (idx[i++] != -1);
        }


        for (std::map<int32_t, Uint_vec>::const_iterator it = indices.begin(); it != indices.end(); ++it)
        {
            Ptr<Material> mat = nullptr;
            if (pMaterials)
            {
                const SbColor* diffuse = pMaterials->diffuseColor.getValues(0);
                const float* tranparency = pMaterials->transparency.getValues(0);

                RGBA rgba( diffuse[it->first][0],
                           diffuse[it->first][1],
                           diffuse[it->first][2],
                           1.f-tranparency[it->first]);

                mat = Material::create( rgba );
            }

            s->addGeometry( mat, Geometry::create( Geometry::TRIANGLES, pVB, it->second ));
        }

        return s;
    }

    Ptr<SceneNode> traverseGraph(SoNode* node)
    {
        std::map<uint32_t, Ptr<SceneNode> >::iterator it = m_pMapNodes->find(node->getNodeId());
        if (it != m_pMapNodes->end())
            return it->second;

        if (SoGroup* group = dynamic_cast<SoGroup*>(node))
        {
            Ptr<GroupNode> g = GroupNode::create();
            (*m_pMapNodes)[node->getNodeId()] = g;

            SbViewportRegion vpr;
            SoGetMatrixAction gma(vpr);
            gma.apply(node); //tra->getMatrix(&gma);

            g->setTransform( reinterpret_cast<const Matrix&>(gma.getMatrix()) );

            //TODO: Normals, TextureCoords and Textures ...

            SoMaterial* materials = NULL;
            SoCoordinate3* coords = NULL;
            SoIndexedFaceSet* indices = NULL;

            for (int i = 0; i < group->getNumChildren(); i++)
            {
                SoNode* child = group->getChild(i);

                if (coords == NULL && child->isOfType( SoCoordinate3::getClassTypeId() ))
                {
                    coords = dynamic_cast<SoCoordinate3*>(child);
                    continue;
                }

                if (indices == NULL && child->isOfType( SoIndexedFaceSet::getClassTypeId() ))
                {
                    indices = dynamic_cast<SoIndexedFaceSet*>(child);
                    continue;
                }

                if ( child->isOfType( SoMaterialBinding::getClassTypeId() ) )
                    continue;

                if (materials == NULL && child->isOfType( SoMaterial::getClassTypeId() ))
                {
                    materials = dynamic_cast<SoMaterial*>(child);
                    continue;
                }

                g->addChildNodes( traverseGraph( group->getChild(i) ) );
            }

            g->addChildNodes( makeShape(coords, materials, indices) );

            return g;
        }
        else
        {
            return makeShape(node);
        }
    }

    static SbBool ProgressCallback(const SbName & itemid, float fraction, SbBool interruptible, void * userdata)
    {
        std::cout << fraction << std::endl;
        SceneIO::progress_callback progress_callback = *((SceneIO::progress_callback*)userdata);
        progress_callback(fraction);
        return true;
    }

    virtual bool read(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress)
    {
        std::map<uint32_t, Ptr<SceneNode> > NodesMap;
        m_pMapNodes = &NodesMap;
        Ptr<IVertexBuffer> pVB = CreateVertexBuffer( sizeof(Float)*6 );
        m_pVB = pVB.get();

        std::unique_ptr<char> data;
        size_t size = SceneIO::File(sFile).getContent(data);
        if ( size == 0 )
        {
            std::wcerr << L"SceneIO::File::getConent() failed: " << sFile.c_str() << std::endl;
            return false;
        }

        std::string sFileA(sFile.begin(), sFile.end());

        SoDB::init();
        SoNodeKit::init();
        SoInteraction::init();

        SoDB::addProgressCallback(ProgressCallback, &progress);
        SoDB::startNotify();

        SoInput input;
        input.setBuffer( data.get(), size);

        SoSeparator* rootWRLNode = SoDB::readAll(&input);

        input.closeFile();

        if (rootWRLNode)
        {
            rootWRLNode->ref();

            pScene->insertNode( traverseGraph(rootWRLNode) );

            rootWRLNode->unref();
            return true;
        }

        return false;
    }

    virtual std::wstring about() const
    {
        return L"coin3d_loader";
    }
    virtual Uint file_type_count() const
    {
        return 1;
    }
    virtual std::wstring file_type(Uint i) const
    {
        return L"Vrml/Inventor Files";
    }
    virtual std::wstring file_exts(Uint i) const
    {
        return L"*.wrl;*.iv";
    }
    virtual std::wstring rpath() const
    {
        return L"";
    }
    virtual bool canWrite(Uint i) const
    {
        return false;
    }

    virtual bool canRead(Uint i) const
    {
        return true;
    }

    virtual bool write(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress)
    {
        return false;
    }
};

extern "C"
#if defined(_MSC_VER)
    __declspec(dllexport)
#endif
    SceneIO::IPlugIn* XcreatePlugIn()
{
    return new Coin3DLoader();
}
