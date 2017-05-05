#pragma once

#include <memory>
#include <string>

namespace eh
{
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
		
		virtual bool loadScene(std::wstring filePath, Callback* callback = nullptr) = 0;

		virtual bool isValid() = 0;

		virtual IController* control() = 0;
	};

	std::shared_ptr<IViewport> CreateViewport(void* hWindow);
}
