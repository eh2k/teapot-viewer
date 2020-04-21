#pragma once

#include <memory>
#include <string>
#include <vector>
#include "math3d.hpp"
using namespace math3D;

struct IGeometry
{
    virtual void AddVertex(const math3D::Vec3 &p, const math3D::Vec3 &n, const math3D::Vec3 &t) = 0;
};

struct IMaterial
{
    virtual void SetDiffuseColor(float r, float g, float b, float a) = 0;
    virtual void SetAmbientColor(float r, float g, float b, float a) = 0;
    virtual void SetSpecularColor(float r, float g, float b, float a) = 0;
    virtual void SetSpecularFactor(float f) = 0;
    virtual void SetEmissionColor(float r, float g, float b, float a) = 0;

    virtual void SetDiffuseTexture(std::wstring fileName) = 0;
    virtual void SetReflectionTexture(std::wstring fileName) = 0;
    virtual void SetBumpTexture(std::wstring fileName) = 0;

    virtual void ReloadTextures() = 0;
};

struct ISceneNode
{
    virtual ~ISceneNode() {}
};

struct IShapeNode : public ISceneNode
{
    virtual void AddGeometry(std::shared_ptr<IMaterial> material, std::shared_ptr<IGeometry> geometry) = 0;
};

struct IGroupNode : public ISceneNode
{
    virtual void AddChildNode(std::shared_ptr<ISceneNode> childNode) = 0;
};

class ISceneHelper
{
public:
    virtual std::shared_ptr<IGeometry> CreateGeometry() = 0;
    virtual std::shared_ptr<IMaterial> CreateMaterial() = 0;
    virtual std::shared_ptr<IShapeNode> CreateShapeNode() = 0;
    virtual std::shared_ptr<IGroupNode> CreateGroupNode(const math3D::Matrix &transform) = 0;
    virtual std::vector<std::shared_ptr<IMaterial>> GetMaterials(std::shared_ptr<ISceneNode> sceneNode) = 0;
    virtual void progress(float p) = 0;
    virtual void GetFileData(const std::wstring file, std::unique_ptr<char>& ptr, size_t& size) = 0;
};

struct IImportPlugIn
{
    virtual std::wstring GetAboutString() const = 0;
    virtual int GetFileTypeCount() const = 0;
    virtual std::wstring GetFileType(int i) const = 0;
    virtual std::wstring GetFileExtention(int i) const = 0;

    virtual std::shared_ptr<ISceneNode> ReadFile(std::wstring aFile, ISceneHelper *sceneHelper) = 0;
};