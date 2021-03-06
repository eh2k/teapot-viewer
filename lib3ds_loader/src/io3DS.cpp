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

#include <plugin.h>

#include <lib3ds.h>

#include <string.h>
#include <math.h>
#include <map>

class C3DSLoader : public IImportPlugIn
{
private:
    float m_nCount;
    float m_iCount;

public:
    C3DSLoader() : m_nCount(0),
                   m_iCount(0)
    {
    }
    virtual ~C3DSLoader()
    {
    }

    void countNodes(Lib3dsFile *f, Lib3dsNode *first_node)
    {
        Lib3dsNode *p;
        for (p = first_node; p; p = p->next)
        {
            if (p->type == LIB3DS_NODE_MESH_INSTANCE)
            {
                if (Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(f, (Lib3dsNode *)p))
                    m_nCount += mesh->nfaces;

                countNodes(f, p->childs);
            }
        }
    }

    void makeNodes(Lib3dsFile *f, Lib3dsNode *first_node, std::vector<std::shared_ptr<ISceneNode>> &nodes, IScene &sceneHelper)
    {
        Lib3dsNode *p;
        for (p = first_node; p; p = p->next)
        {
            if (p->type == LIB3DS_NODE_MESH_INSTANCE)
            {
                if (auto node = makeNode(f, (Lib3dsMeshInstanceNode *)p, sceneHelper))
                    nodes.push_back(node);

                makeNodes(f, p->childs, nodes, sceneHelper);
            }
        }
    }
    std::shared_ptr<IGroupNode> makeNode(Lib3dsFile *f, Lib3dsMeshInstanceNode *node, IScene &sceneHelper)
    {
        Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(f, (Lib3dsNode *)node);
        if (!mesh || !mesh->vertices)
            return nullptr;

        //fprintf(o, "# object %s\n", node->base.name);
        //fprintf(o, "g %s\n", node->instance_name[0]? node->instance_name : node->base.name);

        std::vector<Vec3> normals(mesh->nvertices);

        for (int i = 0; i < mesh->nfaces; ++i)
        {
            sceneHelper.progress(++m_iCount / m_nCount);

            const Vec3 &va = reinterpret_cast<const Vec3 &>(mesh->vertices[mesh->faces[i].index[0]]);
            const Vec3 &vb = reinterpret_cast<const Vec3 &>(mesh->vertices[mesh->faces[i].index[1]]);
            const Vec3 &vc = reinterpret_cast<const Vec3 &>(mesh->vertices[mesh->faces[i].index[2]]);

            Vec3 n = calcNormal(va, vb, vc);
            normals[mesh->faces[i].index[0]] = (normals[mesh->faces[i].index[0]] + n).normalized();
            normals[mesh->faces[i].index[1]] = (normals[mesh->faces[i].index[1]] + n).normalized();
            normals[mesh->faces[i].index[2]] = (normals[mesh->faces[i].index[2]] + n).normalized();
        }

        std::map<int, std::shared_ptr<IVertexBuffer>> faces;
        Uint_vec edges;

        for (int i = 0; i < mesh->nfaces; ++i)
        {
            auto g = faces[mesh->faces[i].material];
            if (g == nullptr)
            {
                g = sceneHelper.CreateVertexBuffer();
                faces[mesh->faces[i].material] = g;
            }

            const Vec3 &va = reinterpret_cast<const Vec3 &>(mesh->vertices[mesh->faces[i].index[0]]);
            const Vec3 &vb = reinterpret_cast<const Vec3 &>(mesh->vertices[mesh->faces[i].index[1]]);
            const Vec3 &vc = reinterpret_cast<const Vec3 &>(mesh->vertices[mesh->faces[i].index[2]]);

            const Vec3 &na = reinterpret_cast<const Vec3 &>(normals[mesh->faces[i].index[0]]);
            const Vec3 &nb = reinterpret_cast<const Vec3 &>(normals[mesh->faces[i].index[1]]);
            const Vec3 &nc = reinterpret_cast<const Vec3 &>(normals[mesh->faces[i].index[2]]);

            Vec3 ta = Vec3::Null();
            Vec3 tb = Vec3::Null();
            Vec3 tc = Vec3::Null();

            if (mesh->texcos != 0)
            {
                ta.x = mesh->texcos[mesh->faces[i].index[0]][0];
                ta.y = mesh->texcos[mesh->faces[i].index[0]][1];

                tb.x = mesh->texcos[mesh->faces[i].index[1]][0];
                tb.y = mesh->texcos[mesh->faces[i].index[1]][1];

                tc.x = mesh->texcos[mesh->faces[i].index[2]][0];
                tc.y = mesh->texcos[mesh->faces[i].index[2]][1];
            }

            g->AddVertex(va, na, ta);
            g->AddVertex(vb, nb, tb);
            g->AddVertex(vc, nc, tc);
        }

        auto pShape = sceneHelper.CreateShapeNode();

        //if(edges.size() > 0)
        //	pShape->AddGeometry( Material::Black(), Geometry::create(Geometry::LINES, m_pVB, edges) );

        for (auto it = faces.begin(); it != faces.end(); it++)
        {
            if (it->first != -1)
            {
                Lib3dsMaterial *mat = f->materials[it->first];

                auto pMat = sceneHelper.CreateMaterial();

                pMat->SetDiffuseColor(mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], mat->transparency);
                pMat->SetAmbientColor(mat->ambient[0], mat->ambient[1], mat->ambient[2], 0);
                pMat->SetSpecularColor(mat->specular[0], mat->specular[1], mat->specular[2], 0);
                pMat->SetEmissionColor(0, 0, 0, 0);
                pMat->SetSpecularFactor(0);

                if (strlen(mat->texture1_map.name) > 0)
                {
                    // if (pMat->GetDiffuse().isBlack())
                    //     pMat->SetDiffuseColor(RGBA(1, 1, 1));

                    std::string s(mat->texture1_map.name);
                    pMat->SetDiffuseTexture(std::wstring(s.begin(), s.end()));
                }

                //CString sTextureFile2( mat->texture2_map.name );
                //if(!sTextureFile2.IsEmpty())
                //{
                //	pMat->addTexture( Texture::createFromFile( m_sPath + sTextureFile2, true ) );
                //}

                pShape->AddTriangles(pMat, it->second);
            }
            else
                pShape->AddTriangles(sceneHelper.CreateMaterial(), it->second);
        }

        float inv_matrix[4][4], M[4][4];

        lib3ds_matrix_copy(M, node->base.matrix);
        lib3ds_matrix_translate(M, -node->pivot[0], -node->pivot[1], -node->pivot[2]);
        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_mult(M, M, inv_matrix);

        auto g = sceneHelper.CreateGroupNode(reinterpret_cast<Matrix &>(M));
        g->AddChildNode(pShape);
        return g;
    }

    // Interface //

    bool read(std::wstring sFile, IScene *scene) override
    {
        m_nCount = 0;
        m_iCount = 0;

        struct FileIO : public Lib3dsIo
        {
            long m_pos;
            size_t m_size;
            std::unique_ptr<char>& m_pBuff;

            FileIO(std::unique_ptr<char>& data, size_t len):
                m_pBuff(data), m_size(len)
            {
                this->self = this;
                m_pos = 0;

                seek_func = seek_func_impl;
                tell_func = tell_func_impl;
                read_func = read_func_impl;
                write_func = write_func_impl;
                log_func = log_func_impl;
            }

            ~FileIO()
            {
            }

            static long seek_func_impl(void *self, long offset, Lib3dsIoSeek origin)
            {
                switch (origin)
                {
                case LIB3DS_SEEK_SET:
                    reinterpret_cast<FileIO *>(self)->m_pos = offset;
                    break;
                case LIB3DS_SEEK_CUR:
                    reinterpret_cast<FileIO *>(self)->m_pos += offset;
                    break;
                case LIB3DS_SEEK_END:
                    reinterpret_cast<FileIO *>(self)->m_pos = reinterpret_cast<FileIO *>(self)->m_size - offset;
                    break;
                }

                return reinterpret_cast<FileIO *>(self)->m_pos;
            }

            static long tell_func_impl(void *self)
            {
                return reinterpret_cast<FileIO *>(self)->m_pos;
            }

            static size_t read_func_impl(void *self, void *buffer, size_t size)
            {
                memcpy(buffer, &reinterpret_cast<FileIO *>(self)->m_pBuff.get()[reinterpret_cast<FileIO *>(self)->m_pos], size);
                reinterpret_cast<FileIO *>(self)->m_pos += size;
                return size;
            }
            static size_t write_func_impl(void *self, const void *buffer, size_t size)
            {
                return 0;
            }
            static void log_func_impl(void *self, Lib3dsLogLevel level, int indent, const char *msg)
            {
            }

            static Lib3dsFile *lib3ds_file_open(std::unique_ptr<char>& data, size_t len)
            {
                Lib3dsFile *file;

                file = lib3ds_file_new();
                if (!file)
                {
                    return NULL;
                }

                FileIO io(data, len);

                if (io.m_size == 0 || !lib3ds_file_read(file, &io))
                    return NULL;

                return file;
            }
        };

        std::unique_ptr<char> data;
        size_t size;
        scene->GetFileData(sFile, data, size);
        Lib3dsFile *f = FileIO::lib3ds_file_open(data, size);
        if (!f)
            return false;

        if (!f->nodes)
            lib3ds_file_create_nodes_for_meshes(f);

        lib3ds_file_eval(f, 0);

        std::vector<std::shared_ptr<ISceneNode>> nodes;

        countNodes(f, f->nodes);
        makeNodes(f, f->nodes, nodes, *scene);

        auto pScene = scene->CreateGroupNode(Matrix::Identity());

        for (size_t i = 0; i < nodes.size(); i++)
            pScene->AddChildNode(nodes[i]);

        for (int i = 0; i < f->cameras_size; i++)
        {
            if (f->cameras != NULL && f->cameras[i] != NULL)
            {
                Lib3dsCamera *c = f->cameras[i];
                Vec3 pos(c->position[0], c->position[1], c->position[2]);
                Vec3 dir = (Vec3(c->target[0], c->target[1], c->target[2]) - pos).normalized();
                Vec3 up = cross(cross(Vec3(0, 0, 1), dir), dir) * -1.f;

                Float near = 1;
                if (c->near_range > 0.f)
                    near = c->near_range;

                Float h = 2.f * (tan(DEG2RAD(c->fov / 2.f)) * near);
                //pScene->addCamera(Camera::create(c->name, h, h, near, c->far_range, pos, dir, up));
                break;
            }
        }

        lib3ds_file_free(f);

        scene->AddRoot(pScene);
        return true;
    }

    virtual std::wstring about() const
    {
        return L"lib3ds_loader";
    }
    virtual int file_type_count() const
    {
        return 1;
    }
    virtual std::wstring file_type(int i) const
    {
        return L"3D Studio Max Models";
    }
    virtual std::wstring file_exts(int i) const
    {
        return L"*.3ds";
    }
};

extern "C"
#if defined(_MSC_VER)
    __declspec(dllexport)
#endif
    IImportPlugIn *XcreatePlugIn()
    {
        return new C3DSLoader();
    }
