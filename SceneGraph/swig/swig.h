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
		virtual void* Handle() = 0;
	};

	struct IShapeNode : public ISceneNode
	{};

	struct IGroupNode : public ISceneNode
	{
		virtual void AddChildNode(std::shared_ptr<ISceneNode> childNode) = 0;
		static std::shared_ptr<IGroupNode> FromHandle(void* handle);
	};

	class Scene
	{
	public:
		static std::shared_ptr<IGeometry> CreateGeometry();
		static std::shared_ptr<IMaterial> CreateMaterial();
		static std::shared_ptr<IShapeNode> CreateShapeNode(std::shared_ptr<IMaterial> material, std::shared_ptr<IGeometry> geometry);
		static std::shared_ptr<IGroupNode> CreateGroupNode(const math3D::Matrix& transform);
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
		virtual void call(float value)
		{
		}
	};

	struct IViewport
	{
		virtual void setDisplayRect(int x, int y, int dx, int dy) = 0;
		virtual void drawScene() = 0;
		virtual std::string getDriverInfo() = 0;

		virtual bool isValid() = 0;

		virtual IController* control() = 0;

		virtual void setModeFlag(Mode flag, bool enable) = 0;
		virtual bool getModeFlag(Mode flag) = 0;

		virtual int getCameraCount() = 0;
		virtual std::wstring getCameraName(int num) = 0;
		virtual void setCamera(int num) = 0;
		virtual void setScene(std::shared_ptr<ISceneNode> scene) = 0;
	};

	std::shared_ptr<IViewport> CreateViewport(void* hWindow);

	struct IPlugIn
	{
		virtual std::wstring about() const { return L""; }
		virtual int file_type_count() const { return 0; }
		virtual std::wstring file_type(int i) { return L""; }
		virtual std::wstring file_exts(int i) { return L""; }

		virtual bool canWrite(int i) const { return false; }
		virtual bool canRead(int i) const { return false; }

		virtual bool readFile(std::wstring aFile, void* sceneHandle, Callback* callback = nullptr) { return false; }
		virtual bool writeFile(std::wstring sFile, void* sceneHandle, Callback* callback = nullptr) { return false; }
	};

	struct SceneIO
	{
		static void RegisterPlugIn(std::shared_ptr<IPlugIn> plugIn);

		static bool read(std::shared_ptr<IViewport> viewPort, std::wstring filePath, Callback* callback = nullptr);
		static bool write(std::shared_ptr<IViewport> viewPort, std::wstring filePath, Callback* callback = nullptr);
		static std::wstring getFileWildcards(bool read = true);

		static std::wstring getAboutString();
	};


}
