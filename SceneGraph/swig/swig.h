#pragma once

#include <memory>
#include <string>
#include <vector>
#include "..\src\ViewportModes.h"
#include "..\src\math3d.hpp"
#include "..\src\Geometry.h"

namespace swig
{
	struct IGeometry
	{
		virtual void AddVertex(const math3D::Vec3& p, const math3D::Vec3& n, const math3D::Vec3& t) = 0;
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
	};

	struct ISceneNode
	{
		virtual ~ISceneNode() {}
	};

	struct IShapeNode : public ISceneNode
	{};

	struct IGroupNode : public ISceneNode
	{
		virtual void AddChildNode(std::shared_ptr<ISceneNode> childNode) = 0;
	};

	class Scene
	{
	public:
		static std::shared_ptr<IGeometry> CreateGeometry();
		static std::shared_ptr<IMaterial> CreateMaterial();
		static std::shared_ptr<IShapeNode> CreateShapeNode(std::shared_ptr<IMaterial> material, std::shared_ptr<IGeometry> geometry);
		static std::shared_ptr<IGroupNode> CreateGroupNode(const math3D::Matrix& transform);

		static std::shared_ptr<IGroupNode> TryGetGroupNodeFromHandle(void* handle);
		static void* NodeToHandle(std::shared_ptr<ISceneNode>);
	};

	///////////////

	struct IController
	{
		typedef int Flags;

		static const Flags LBUTTON = 0x0001;
		static const Flags RBUTTON = 0x0002;
		static const Flags SHIFT = 0x0004;
		static const Flags CONTROL = 0x0008;
		static const Flags MBUTTON = 0x0010;

		virtual void OnMouseMove(Flags nFlags, int x, int y) = 0;
		virtual void OnMouseDown(Flags nFlags, int x, int y) = 0;
		virtual void OnMouseUp(Flags nFlags, int x, int y) = 0;
		virtual void OnMouseWheel(Flags nFlags, short zDelta, int x, int y) = 0;
		virtual void OnKeyDown(int nKeyCode) = 0;
		virtual void Animate() = 0;
	};

	struct Callback
	{
		virtual ~Callback() { }
		virtual void Call(float value)
		{
		}
	};

	struct IViewport
	{
		virtual void SetDisplayRect(int x, int y, int dx, int dy) = 0;
		virtual void DrawScene() = 0;
		virtual std::string GetDriverInfo() = 0;

		virtual bool IsValid() = 0;

		virtual IController* Control() = 0;

		virtual void SetModeFlag(Mode flag, bool enable) = 0;
		virtual bool GetModeFlag(Mode flag) = 0;

		virtual int GetCameraCount() = 0;
		virtual std::wstring GetCameraName(int num) = 0;
		virtual void SetCamera(int num) = 0;

		virtual void SetScene(std::shared_ptr<ISceneNode> scene) = 0;
	};

	std::shared_ptr<IViewport> CreateViewport(void* hWindow);

	struct IPlugIn
	{
		virtual std::wstring GetAboutString() const = 0;
		virtual int GetFileTypeCount() const = 0;
		virtual std::wstring GetFileType(int i) = 0;
		virtual std::wstring GetFileExtention(int i) = 0;

		virtual bool ReadFile(std::wstring aFile, void* sceneHandle, Callback* callback = nullptr) = 0;
	};

	struct SceneIO
	{
		static void RegisterPlugIn(std::shared_ptr<IPlugIn> plugIn);

		static std::shared_ptr<IGroupNode> TryRead(std::wstring filePath, Callback* callback = nullptr);

		static std::wstring GetFileWildcards(bool read = true);
		static std::wstring GetAboutString();
	};
}
