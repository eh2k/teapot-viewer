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

#include <iostream>
#include <unordered_map>

#include <FCollada.h>

#include <FCDocument/FCDocument.h>
#include <FCDocument/FCDLibrary.h>
#include <FCDocument/FCDGeometry.h>
#include <FCDocument/FCDGeometryMesh.h>
#include <FCDocument/FCDGeometryPolygonsTools.h>
#include <FCDocument/FCDGeometryPolygons.h>
#include <FCDocument/FCDGeometryPolygonsInput.h>
#include <FCDocument/FCDGeometrySource.h>
#include <FCDocument/FCDMaterial.h>
#include <FCDocument/FCDEffect.h>
#include <FCDocument/FCDEffectProfile.h>
#include <FCDocument/FCDEffectStandard.h>
#include <FCDocument/FCDSceneNode.h>
#include <FCDocument/FCDTransform.h>
#include <FCDocument/FCDGeometryInstance.h>
#include <FCDocument/FCDMaterialInstance.h>
#include <FCDocument/FCDTexture.h>
#include <FCDocument/FCDImage.h>
#include <FCDocument/FCDCamera.h>
#include <FCDocument/FCDAnimated.h>
#include <FCDocument/FCDAnimationCurve.h>

#include "FUtils/FUFileManager.h"
#include "FColladaPlugin.h"
#include <map>

#include <plugin.h>
#include <cassert>

struct RGBA
{
    float r, g, b, a;
};

class COLLADALoader : public IImportPlugIn
{
private:
    typedef std::unordered_map<std::string, std::pair<FCDCamera *, Matrix>> CAMERAS;
    CAMERAS m_cams;

    IScene *scene;
    std::shared_ptr<IVertexBuffer> m_pVB;
    std::unordered_map<std::string, std::shared_ptr<IMaterial>> m_materials;

    typedef std::unordered_map<IShapeNode::TYPE, Uint_vec> PRIM_INDICES;
    typedef std::unordered_map<std::wstring, PRIM_INDICES> MAT_PRIM_INDICES;
    std::unordered_map<std::string, MAT_PRIM_INDICES> m_geometry;

    void addPolygons(FCDGeometryPolygons *pPolys, MAT_PRIM_INDICES &matprims)
    {

        // indices to vertex
        FCDGeometryPolygonsInput *pi = pPolys->FindInput(FUDaeGeometryInput::POSITION);
        FCDGeometryPolygonsInput *ni = pPolys->FindInput(FUDaeGeometryInput::NORMAL);
        FCDGeometryPolygonsInput *ti = pPolys->FindInput(FUDaeGeometryInput::TEXCOORD);

        // source of vertex
        FCDGeometrySource *positions = pPolys->GetParent()->FindSourceByType(FUDaeGeometryInput::POSITION);
        FCDGeometrySource *normals = pPolys->GetParent()->FindSourceByType(FUDaeGeometryInput::NORMAL);
        FCDGeometrySource *texcoords = pPolys->GetParent()->FindSourceByType(FUDaeGeometryInput::TEXCOORD);

		auto PrimMode = IShapeNode::TRIANGLES;
		switch(pPolys->GetPrimitiveType())
		{
		case FCDGeometryPolygons::LINES:
			PrimMode = IShapeNode::LINES;
			break;
		case FCDGeometryPolygons::LINE_STRIPS:
			PrimMode = IShapeNode::LINE_STRIP;
			break;
		case FCDGeometryPolygons::POLYGONS:
			PrimMode = IShapeNode::TRIANGLES;
			break;
		case FCDGeometryPolygons::TRIANGLE_FANS:
			PrimMode = IShapeNode::TRIANGLE_FAN;
			break;
		case FCDGeometryPolygons::TRIANGLE_STRIPS:
			PrimMode = IShapeNode::TRIANGLE_STRIP;
			break;
		case FCDGeometryPolygons::POINTS:
			PrimMode = IShapeNode::POINTS;
			break;
		}

		Uint_vec& idx = matprims[pPolys->GetMaterialSemantic().c_str()][PrimMode];

        for (size_t i = 0; i < pi->GetIndexCount(); i++)
        {
            uint32 _pi = (uint32)pi->GetIndices()[i];

            const Vec3 *v = NULL;
            const Vec3 *n = NULL;

            v = reinterpret_cast<math3D::Vec3 *>(&positions->GetData()[_pi * 3]);
            if (normals)
            {
                uint32 _ni = (uint32)ni->GetIndices()[i];
                n = reinterpret_cast<math3D::Vec3 *>(&normals->GetData()[_ni * 3]);
            }
            else
                n = &math3D::Vec3::Null();

            if (texcoords)
            {
                uint32 _ti = (uint32)ti->GetIndices()[i];
                Vec3 t;
                t.x = texcoords->GetData()[_ti * texcoords->GetStride()];
                t.y = texcoords->GetData()[_ti * texcoords->GetStride() + 1];

                idx.push_back(m_pVB->AddVertex(*v, *n, t));
            }
            else
            {
                idx.push_back(m_pVB->AddVertex(*v, *n, Vec3::Null()));
            }
        }
    }
    void addMesh(FCDGeometryMesh *pMesh)
    {
        if (!pMesh->IsTriangles())
            FCDGeometryPolygonsTools::Triangulate(pMesh);

        for (size_t j = 0; j < pMesh->GetPolygonsCount(); j++)
        {
            if (FCDGeometryPolygons *pPolys = pMesh->GetPolygons(j))
                addPolygons(pPolys, m_geometry[pMesh->GetDaeId().c_str()]);
        }
    }

    const Matrix &getTransform(FCDSceneNode *pNode, std::vector<Matrix> &transforms)
    {
        if (pNode != NULL)
            for (size_t i = 0; i < pNode->GetTransformCount(); i++)
            {
                FCDTransform *t = pNode->GetTransform(i);
                if (t->IsAnimated())
                {
                    float start_time = t->GetDocument()->GetStartTime();
                    float end_time = t->GetDocument()->GetEndTime();

                    transforms.resize((size_t)((end_time - start_time) * 30.f + 1), transforms[0]);
                    for (size_t i = 0; i < transforms.size(); i++)
                    {
                        FCDAnimated *a = t->GetAnimated();
                        a->Evaluate(start_time + (1.f / 30.f) * i);
                        auto tmp = t->ToMatrix();
                        transforms[i] = reinterpret_cast<const Matrix &>(tmp.m[0][0]) * transforms[i];
                    }
                }
                else
                {
                    auto tmp = t->ToMatrix();
                    for (size_t i = 0; i < transforms.size(); i++)
                        transforms[i] = reinterpret_cast<const Matrix &>(tmp.m[0][0]) * transforms[i];
                }
            }

        return transforms[0];
    }

    int traverseSG(FCDSceneNode *pNode, std::shared_ptr<IGroupNode> pParentGroup, const Matrix &_tra = Matrix::Identity())
    {
        int geo_count = 0;

        std::vector<Matrix> transforms(1, Matrix::Identity());
        Matrix tra = _tra * getTransform(pNode, transforms);

        //auto pGroup = GroupNode::createAnimated( SceneNodeVector(), transforms );
        auto pGroup = scene->CreateGroupNode(transforms[0]);
        auto pShape = scene->CreateShapeNode();

        for (size_t i = 0; i < pNode->GetInstanceCount(); i++)
        {
            FCDEntityInstance *pInst = pNode->GetInstance(i);
            FCDEntity *pEntity = pInst->GetEntity();
            std::string id = pEntity->GetDaeId().c_str();

            if (m_cams.find(id.c_str()) != m_cams.end())
                m_cams[id.c_str()].second = tra;

            if (pInst->GetType() != FCDEntityInstance::GEOMETRY)
                continue;

            FCDGeometryInstance *pGeoInst = dynamic_cast<FCDGeometryInstance *>(pInst);

            //assert(m_geometry.find(id) != m_geometry.end());

            for (MAT_PRIM_INDICES::const_iterator it = m_geometry[id].begin(),
                                                  end = m_geometry[id].end();
                 it != end; it++)
            {
                std::shared_ptr<IMaterial> pMat = nullptr;

                for (size_t k = 0; k < pGeoInst->GetMaterialInstanceCount(); k++)
                {
                    FCDMaterialInstance *pMatInst = pGeoInst->GetMaterialInstance(k);
                    std::string matid = pMatInst->GetMaterial()->GetDaeId().c_str();

                    if (std::wstring(pMatInst->GetSemantic().c_str()) == it->first)
                    {
                        pMat = m_materials[matid];
                        break;
                    }
                }

                for (PRIM_INDICES::const_iterator piit = it->second.begin(); piit != it->second.end(); piit++)
                {
                    pShape->AddGeometry(pMat, piit->first, m_pVB, piit->second);
                    geo_count++;
                }
            }
        }

        if (geo_count > 0)
            pGroup->AddChildNode(pShape);

        for (size_t i = 0; i < pNode->GetChildrenCount(); i++)
            geo_count += traverseSG(pNode->GetChild(i), pGroup, tra);

        if (geo_count > 0)
            pParentGroup->AddChildNode(pGroup);

        return geo_count;
    }

public:
    COLLADALoader() : m_pVB(nullptr)
    {
        FCollada::Initialize();
    }
    virtual ~COLLADALoader()
    {
        FCollada::Release();
    }

    bool read(std::wstring sFile, IScene *scene) override
    {
        this->scene = scene;
        m_pVB = scene->CreateVertexBuffer(); // sizeof(Vec3)*2 + sizeof(Float)*2 );

        FCDocument doc;
        std::unique_ptr<char> data;
        size_t size = 0;
        scene->GetFileData(sFile, data, size);
        if (size == 0)
        {
            std::wcerr << L"FaFile.getContent(data.get()) != size" << std::endl;
            return false;
        }

        if (FCollada::LoadDocumentFromMemory(sFile.c_str(), &doc, data.get(), size) == false)
        {
            std::wcerr << L"FCollada::LoadDocumentFromMemory(" << sFile << L") failed" << std::endl;
            return false;
        }

        //const Vec3& up = reinterpret_cast<const Vec3&>(doc.GetAsset()->GetUpAxis());
        //std::cout << "UpAxis:" << up;

        if (FCDCameraLibrary *camlib = doc.GetCameraLibrary())
        {
            for (size_t i = 0; i < camlib->GetEntityCount(); i++)
            {
                FCDCamera *pFCam = camlib->GetEntity(i);
                m_cams[pFCam->GetDaeId().c_str()].first = pFCam;
            }
        }

        // Materials

        if (FCDMaterialLibrary *materiallib = doc.GetMaterialLibrary())
        {
            for (size_t i = 0; i < materiallib->GetEntityCount(); i++)
            {
                FCDMaterial *pFMat = materiallib->GetEntity(i);

                FCDEffect *pFx = pFMat->GetEffect();

                FCDEffectStandard *pProfile = dynamic_cast<FCDEffectStandard *>(pFx->FindProfile(FUDaeProfileType::COMMON));

                if (pProfile)
                {
                    auto pMat = scene->CreateMaterial();
                    m_materials[pFMat->GetDaeId().c_str()] = pMat;
                    RGBA diffuse = reinterpret_cast<const RGBA &>(pProfile->GetDiffuseColor());
                    if (pProfile->GetTransparencyMode() != FCDEffectStandard::/*TransparencyMode::*/ A_ONE)
                        diffuse.a = diffuse.a * 0.5f * pProfile->GetTranslucencyFactor();
                    else
                        diffuse.a = 1.f - pProfile->GetTranslucencyFactor();

                    //if(diffuse.A() < 0.4)
                    //	diffuse.A() = 0.2;

                    pMat->SetDiffuseColor(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
                    auto a = reinterpret_cast<const RGBA &>(pProfile->GetAmbientColor());
                    pMat->SetAmbientColor(a.r, a.g, a.b, a.a);
                    auto s = reinterpret_cast<const RGBA &>(pProfile->GetSpecularColor());
                    pMat->SetSpecularColor(s.r, s.g, s.b, s.a);
                    auto e = reinterpret_cast<const RGBA &>(pProfile->GetEmissionColor());
                    pMat->SetEmissionColor(e.r, e.g, e.b, e.a);
                    pMat->SetSpecularFactor(pProfile->GetSpecularFactor());

                    for (size_t j = 0; j < pProfile->GetTextureCount(FUDaeTextureChannel::DIFFUSE); j++)
                    {
                        if (FCDTexture *pTexture = pProfile->GetTexture(FUDaeTextureChannel::DIFFUSE, j))
                        {
                            if (FCDImage *pImage = pTexture->GetImage())
                                pMat->SetDiffuseTexture(pImage->GetFilename().c_str());
                        }
                    }
                    for (size_t j = 0; j < pProfile->GetTextureCount(FUDaeTextureChannel::REFLECTION); j++)
                    {
                        if (FCDTexture *pTexture = pProfile->GetTexture(FUDaeTextureChannel::REFLECTION, j))
                        {
                            if (FCDImage *pImage = pTexture->GetImage())
                                pMat->SetReflectionTexture(pImage->GetFilename().c_str());
                        }
                    }
                }
            }
        }

        if (FCDGeometryLibrary *geolib = doc.GetGeometryLibrary())
        {
            for (size_t i = 0; i < geolib->GetEntityCount(); i++)
            {
                scene->progress(((float)i) / geolib->GetEntityCount());

                FCDGeometry *pGeo = geolib->GetEntity(i);

                if (pGeo->IsMesh())
                    addMesh(pGeo->GetMesh());

                if (pGeo->IsPSurface())
                {
                    std::cerr << "pGeo->IsPSurface()" << std::endl;
                }

                if (pGeo->IsSpline())
                {
                    std::cerr << "pGeo->IsSpline()" << std::endl;
                }
            }
        }

        FCDSceneNode *root = doc.GetVisualSceneRoot();

        auto g = scene->CreateGroupNode(Matrix::Identity());

        if (traverseSG(root, g) > 0)
        {
            scene->AddRoot(g);

            for (CAMERAS::const_iterator it = m_cams.begin(); it != m_cams.end(); ++it)
            {
                FCDCamera *cam = m_cams[it->first].first;
                Float aspect = cam->GetAspectRatio();
                Float farZ = cam->GetFarZ();
                Float nearZ = cam->GetNearZ();
                Matrix transform = m_cams[it->first].second;

                Float fovY = 0;

                if (cam->HasVerticalFov())
                    fovY = cam->GetFovY();
                else
                    fovY = aspect * cam->GetFovX();

                if (fequal(aspect, 0))
                    aspect = 1.f;

                if (fequal(fovY, 0))
                    fovY = 45.f;

                Float h = 2.f * (tan(DEG2RAD(fovY / 2.f)) * nearZ);
                Float w = aspect * h;

                assert(!"TODO");
                //pScene->addCamera(Camera::create(it->first, w, h, nearZ, farZ, transform.getInverted()));
            }
        }
        else
        {
            for (auto id = m_geometry.cbegin(); id != m_geometry.cend(); id++)
                for (MAT_PRIM_INDICES::const_iterator it = m_geometry[id->first].begin(),
                                                      end = m_geometry[id->first].end();
                     it != end; it++)
                {
                    for (PRIM_INDICES::const_iterator piit = it->second.begin(); piit != it->second.end(); piit++)
                    {
                        assert(!"TODO");
                        //Ptr<Material> pMat = Material::Blue();
                        //pScene->insertNode(ShapeNode::create(pMat, Geometry::create(piit->first, m_pVB, piit->second)));
                    }
                }
        }

        m_cams.clear();
        m_geometry.clear();
        m_materials.clear();
        m_pVB = nullptr;

        return true;
    }

    virtual std::wstring about() const
    {
        return L"Collada Loader";
    }
    virtual int file_type_count() const
    {
        return 1;
    }
    virtual std::wstring file_type(int i) const
    {
        return L"Collada Models";
    }
    virtual std::wstring file_exts(int i) const
    {
        return L"*.dae;*.xml";
    }
};

extern "C"
{
#if defined(WIN32)
    __declspec(dllexport)
#endif
        IImportPlugIn *XcreatePlugIn()
    {
        return new COLLADALoader();
    }
}
