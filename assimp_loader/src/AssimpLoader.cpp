/****************************************************************************
* Copyright (C) 2007-2017 by E.Heidt  http://teapot-viewer.sourceforge.net/ *
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

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <plugin.h>

#ifdef __cpp_lib_filesystem
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std
{
using namespace std::experimental;
}
#endif

struct AssimpLoader
    : public IImportPlugIn
{
    const C_STRUCT aiScene *_assimp = nullptr;
    std::shared_ptr<IGroupNode> _root = nullptr;
    std::filesystem::path _directory;
    std::vector<std::string> _fileTypes;

    AssimpLoader()
    {
        aiString list;
        aiGetExtensionList(&list);

        std::string token;
        std::istringstream tokenStream(list.C_Str());
        while (std::getline(tokenStream, token, ';'))
        {
            if(token == "*.stp" ||
               token == "*.step" ||
               token == "*.3ds")
               {
                   continue;
               }
            _fileTypes.push_back(token);
        }
    }

    virtual std::wstring about() const
    {
        auto s = std::string("assimp_loader ");
        return std::wstring(s.begin(), s.end());
    }
    virtual int file_type_count() const
    {
        return _fileTypes.size();
    }
    virtual std::wstring file_type(int i) const
    {
        auto tmp = std::string(&_fileTypes[i][2]);
        for (int i = 0; i < tmp.length(); i++)
            tmp[i] = toupper(tmp[i]);

        tmp += " File";
        return std::wstring(tmp.begin(), tmp.end());
    }
    virtual std::wstring file_exts(int i) const
    {
        auto tmp = _fileTypes[i];
        return std::wstring(tmp.begin(), tmp.end());
    }

    bool read(std::wstring wFile, IScene *scene) override
    {
        try
        {
            std::filesystem::path file(wFile);
            _directory = file.parent_path();

            std::unique_ptr<char> data;
            size_t size = 0;
            scene->GetFileData(wFile, data, size);

            //_assimp = aiImportFile(file.string().c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
            _assimp = aiImportFileFromMemory(data.get(), size, aiProcessPreset_TargetRealtime_MaxQuality, "");

            auto root = Translate(scene, _assimp->mRootNode);
            scene->AddRoot(root);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

private:
    void ApplyMaterial(std::shared_ptr<IMaterial> m, const C_STRUCT aiMaterial *mtl)
    {
        C_STRUCT aiColor4D diffuse;
        C_STRUCT aiColor4D specular;
        C_STRUCT aiColor4D ambient;
        C_STRUCT aiColor4D emission;
        ai_real shininess, strength;
        int two_sided;
        int wireframe;
        unsigned int max;

        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            m->SetDiffuseColor(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
        else
            m->SetDiffuseColor(0.8f, 0.8f, 0.8f, 1.0f);

        if (mtl->Get(AI_MATKEY_OPACITY, diffuse.a) == AI_SUCCESS)
        {
            m->SetDiffuseColor(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
        }

        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
            m->SetSpecularColor(specular.r, specular.g, specular.b, specular.a);
        else
            m->SetSpecularColor(0.0f, 0.0f, 0.0f, 1.0f);

        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
            m->SetAmbientColor(ambient.r, ambient.g, ambient.b, ambient.a);
        else
            m->SetAmbientColor(0.2f, 0.2f, 0.2f, 1.0f);

        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
            m->SetEmissionColor(emission.r, emission.g, emission.b, emission.a);
        else
            m->SetEmissionColor(0.0f, 0.0f, 0.0f, 1.0f);

        max = 1;
        int ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
        if (ret1 == AI_SUCCESS)
        {
            max = 1;
            int ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
            if (ret2 == AI_SUCCESS)
                m->SetSpecularFactor(shininess * strength); //GL_SHININESS
            else
                m->SetSpecularFactor(shininess); //GL_SHININESS
        }
        else
        {
            m->SetSpecularFactor(0);
            m->SetSpecularColor(0.0f, 0.0f, 0.0f, 0.0f);
        }

        int texIndex = 0;
        aiReturn texFound = AI_SUCCESS;
        aiString path; // filename
        while (texFound == AI_SUCCESS)
        {
            texFound = mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
            texIndex++;

            if (texFound == AI_SUCCESS)
            {
                auto texture = _directory / path.C_Str();
                m->SetDiffuseTexture(texture.wstring());
                break;
            }
        }
        

        // if (material.HasTextureReflection)
        //     mat.SetReflectionTexture(System.IO.Path.Combine(_directory, material.TextureReflection.FilePath));

        // if (material.HasTextureNormal)
        //     mat.SetBumpTexture(System.IO.Path.Combine(_directory, material.TextureNormal.FilePath));
    }

    std::shared_ptr<IGroupNode> Translate(IScene *scene, const C_STRUCT aiNode *nd)
    {
        auto m = nd->mTransformation;
        auto t = math3D::Matrix(m.a1, m.b1, m.c1, m.d1,
                                m.a2, m.b2, m.c2, m.d2,
                                m.a3, m.b3, m.c3, m.d3,
                                m.a4, m.b4, m.c4, m.d4);

        auto group = scene->CreateGroupNode(t);
        auto shape = scene->CreateShapeNode();

        for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
        {
            const C_STRUCT aiMesh *mesh = _assimp->mMeshes[nd->mMeshes[n]];
            auto m = scene->CreateMaterial();
            ApplyMaterial(m, _assimp->mMaterials[mesh->mMaterialIndex]);

            auto vb = scene->CreateVertexBuffer();
            bool hasTextureCoords = mesh->HasTextureCoords(0);

            for (unsigned int t = 0; t < mesh->mNumFaces; ++t)
            {
                const C_STRUCT aiFace *face = &mesh->mFaces[t];

                for (unsigned int i = 0; i < face->mNumIndices; i++)
                {
                    math3D::Vec3 p;
                    math3D::Vec3 n;
                    math3D::Vec3 t;

                    int index = face->mIndices[i];
                    // if (mesh->mColors[0] != NULL)
                    //     glColor4fv((GLfloat *)&mesh->mColors[0][index]);

                    p = *reinterpret_cast<const math3D::Vec3 *>(&mesh->mVertices[index].x);

                    if (hasTextureCoords)
                    {
                        t.x = mesh->mTextureCoords[0][index].x;
                        t.y = mesh->mTextureCoords[0][index].y;
                    }

                    if (mesh->mNormals != NULL)
                    {
                        n = reinterpret_cast<const math3D::Vec3&>(mesh->mNormals[index].x).normalized();
                    }

                    vb->AddVertex(p, n, t);
                }
            }

            shape->AddTriangles(m, vb);
        }

        if (nd->mNumMeshes > 0)
            group->AddChildNode(shape);

        /* draw all children */
        for (unsigned int n = 0; n < nd->mNumChildren; ++n)
        {
            auto c = Translate(scene, nd->mChildren[n]);
            group->AddChildNode(c);
        }

        return group;
    }
};

extern "C"
#if defined(_MSC_VER)
    __declspec(dllexport)
#endif
        IImportPlugIn *XcreatePlugIn()
{
    return new AssimpLoader();
}
