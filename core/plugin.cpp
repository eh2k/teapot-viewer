
#include <memory>
#include <map>
#include <IDriver.h>
#include <Viewport.h>
#include <Scene.h>
#include <SceneIO.h>
#include <Controller.h>
#include <VertexBuffer.h>
#include "PlugIn.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

struct Geometry : public IGeometry
{
    eh::Ptr<eh::IVertexBuffer> _p = nullptr;
    eh::Uint_vec _indices;

    Geometry(eh::Ptr<eh::IVertexBuffer> p) : _p(p)
    {
    }

    void AddVertex(const math3D::Vec3 &p, const math3D::Vec3 &n, const math3D::Vec3 &t) override
    {
        _indices.push_back(_p->addVertex(p, n, t));
    }
};

struct Material : public IMaterial
{
    eh::Ptr<eh::Material> _p = nullptr;

    Material(eh::Ptr<eh::Material> p) : _p(p)
    {
    }

    void SetDiffuseColor(float r, float g, float b, float a) override
    {
        _p->setDiffuse(eh::RGBA(r, g, b, a));
    }

    void SetAmbientColor(float r, float g, float b, float a) override
    {
        _p->setAmbient(eh::RGBA(r, g, b, a));
    }
    void SetSpecularColor(float r, float g, float b, float a) override
    {
        _p->setSpecular(eh::RGBA(r, g, b, a));
    }
    void SetSpecularFactor(float f) override
    {
        _p->setSpecularFactor(f);
    }
    void SetEmissionColor(float r, float g, float b, float a) override
    {
        _p->setEmission(eh::RGBA(r, g, b, a));
    }

    void SetDiffuseTexture(std::wstring fileName) override
    {
        _p->setTexture(eh::Texture::createFromFile(fileName));
    }

    void SetReflectionTexture(std::wstring fileName) override
    {
        _p->setReflTexture(eh::Texture::createFromFile(fileName));
    }

    void SetBumpTexture(std::wstring fileName) override
    {
        _p->setBumpTexture(eh::Texture::createFromFile(fileName));
    }

    void ReloadTextures() override
    {

        if (auto t = _p->getTexture())
            t->m_resource.reset();

        if (auto t = _p->getReflTexture())
            t->m_resource.reset();

        if (auto t = _p->getOpacTexture())
            t->m_resource.reset();

        if (auto t = _p->getBumpTexture())
            t->m_resource.reset();
    }
};

struct ShapeNode : public IShapeNode
{
    eh::Ptr<eh::ShapeNode> _p = nullptr;

    void AddGeometry(std::shared_ptr<IMaterial> material, std::shared_ptr<IGeometry> geometry) override
    {
        auto m = (Material *)material.get();
        auto g = (Geometry *)geometry.get();
        _p->addGeometry(m->_p, eh::Geometry::create(eh::Geometry::TRIANGLES, g->_p, g->_indices));
    }

    ShapeNode(eh::Ptr<eh::ShapeNode> p) : _p(p)
    {
    }
};

struct GroupNode : public IGroupNode
{
    eh::Ptr<eh::GroupNode> _p = nullptr;

    GroupNode(eh::Ptr<eh::GroupNode> p) : _p(p)
    {
    }

    void AddChildNode(std::shared_ptr<ISceneNode> childNode) override
    {
        ISceneNode *p = childNode.get();
        if (auto pp = dynamic_cast<ShapeNode *>(p))
            _p->addChildNodes(pp->_p);
        else if (auto pp = dynamic_cast<GroupNode *>(p))
            _p->addChildNodes(pp->_p);
    }
};

struct SceneHelper : ISceneHelper
{
    void progress(float p) override
    {
        return;
    }

    void GetFileData(const std::wstring file, std::unique_ptr<char> &ptr, size_t &size) override
    {
        eh::SceneIO::File f(file);
        size = f.getContent(ptr);
    }

    std::shared_ptr<IGeometry> CreateGeometry() override
    {
        return std::make_shared<Geometry>(eh::CreateVertexBuffer(sizeof(eh::Vec3) * 2 + sizeof(eh::Float) * 2));
    }

    std::shared_ptr<IMaterial> CreateMaterial() override
    {
        return std::make_shared<Material>(eh::Material::create());
    }

    std::shared_ptr<IShapeNode> CreateShapeNode() override
    {
        return std::make_shared<ShapeNode>(eh::ShapeNode::create());
    }

    std::shared_ptr<IGroupNode> CreateGroupNode(const math3D::Matrix &transform) override
    {
        return std::make_shared<GroupNode>(eh::GroupNode::create(eh::SceneNodeVector(), transform));
    }

    std::vector<std::shared_ptr<IMaterial>> GetMaterials(std::shared_ptr<ISceneNode> sceneNode) override
    {
        struct RoloadTextureVisitor : eh::IVisitor
        {
            std::vector<std::shared_ptr<IMaterial>> _materials;

            void visit(eh::Geometry &node) override
            {
            }
            void visit(eh::ShapeNode &node) override
            {
                for (auto it = node.GeometryBegin(); it != node.GeometryEnd(); it++)
                {
                    _materials.push_back(std::make_shared<Material>(it.getMaterial()));
                }
            }
            void visit(eh::GroupNode &node) override
            {
                for (auto n : node.getChildNodes())
                    n->accept(*this);
            }
        } tv;

        ISceneNode *p = sceneNode.get();
        if (auto pp = dynamic_cast<ShapeNode *>(p))
            pp->_p->accept(tv);
        else if (auto pp = dynamic_cast<GroupNode *>(p))
            pp->_p->accept(tv);

        return tv._materials;
    }
};

ISceneHelper &GetSceneHelper()
{
    static SceneHelper instance;
    return instance;
}

class XPlugIn : public eh::SceneIO::IPlugIn
{
public:
    IImportPlugIn *_plugIn;
    XPlugIn(IImportPlugIn *plugIn) : _plugIn(plugIn)
    {
    }

    virtual ~XPlugIn()
    {
    }
    virtual std::wstring about() const
    {
        return _plugIn->GetAboutString();
    }
    virtual Uint file_type_count() const
    {
        return _plugIn->GetFileTypeCount();
    }
    virtual std::wstring file_type(Uint i) const
    {
        return _plugIn->GetFileType(i);
    }
    virtual std::wstring file_exts(Uint i) const
    {
        return _plugIn->GetFileExtention(i);
    }
    virtual bool canWrite(Uint i) const
    {
        return false;
    }
    virtual bool canRead(Uint i) const
    {
        return true;
    }

    virtual bool read(const std::wstring &aFile, eh::Ptr<eh::Scene> pScene, eh::SceneIO::progress_callback &progress)
    {
        auto g = _plugIn->ReadFile(aFile, &GetSceneHelper());
        pScene->insertNode(dynamic_cast<GroupNode *>(g.get())->_p);
        return true;
    }

    virtual bool write(const std::wstring &sFile, eh::Ptr<eh::Scene> pScene, eh::SceneIO::progress_callback &progress)
    {
        return false;
    }
};

eh::SceneIO::IPlugIn* XXX(IImportPlugIn* p)
{
    return new XPlugIn(p);
}