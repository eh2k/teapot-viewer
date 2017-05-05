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

#pragma once
#include <wx/wx.h>
#include <Viewport.h>
#include <Controller.h>
#include <Scene.h>

using namespace eh;

template<class T = wxWindow >
class Base3DWnd: public T
{
private:
	wxTimer m_aTimer;
	Viewport m_aViewport;
public:

	Base3DWnd(wxWindow* parent):
		T(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize),
		m_aTimer(this, wxID_ANY),
		m_aViewport(NULL)
	{
		Connect(wxEVT_TIMER, wxTimerEventHandler(Base3DWnd::OnTimer));

		Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(Base3DWnd::OnMouseEvent));
		Connect(wxEVT_LEFT_UP, wxMouseEventHandler(Base3DWnd::OnMouseEvent));
		Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(Base3DWnd::OnMouseEvent));
		Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(Base3DWnd::OnMouseEvent));
		Connect(wxEVT_MOTION, wxMouseEventHandler(Base3DWnd::OnMouseEvent));
		Connect(wxEVT_MOUSEWHEEL, wxMouseEventHandler(Base3DWnd::OnMouseEvent));
		Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Base3DWnd::OnKeyEvent));
	}
	virtual ~Base3DWnd()
	{
	}

	void ResetView()
	{
		if (m_aViewport.getDriver() == NULL)
			return;

		Refresh();

		if (m_aViewport.getScene()->isAnimated())
			m_aTimer.Start(20, true);
		else
			m_aTimer.Stop();
	}

	Viewport& GetViewport()
	{
		return m_aViewport;
	}

	void OnTimer(wxTimerEvent& event)
	{
		m_aViewport.control().Animate();

		if (!m_aViewport.isValid())
			this->Refresh();

		m_aTimer.Start(20, true);
	}

	void OnKeyEvent(wxKeyEvent& event)
	{
		GetViewport().control().OnKeyDown( event.GetUnicodeKey() );
		Refresh();
	}

	void OnMouseEvent(wxMouseEvent& event)
	{
		if (event.LeftDown() || event.RightDown())
		{
			GetViewport().control().OnMouseDown(Controller::LBUTTON,
					event.GetX(), event.GetY());
#if defined(_MSC_VER)
			::SetCapture((HWND)this->GetHWND());
#endif
			Refresh();
		}
		else if (event.LeftUp() || event.RightDown())
		{
#if defined(_MSC_VER)
			if ( ::GetCapture() == (HWND)this->GetHWND() )
#endif
			{
				GetViewport().control().OnMouseUp(
						Controller::LBUTTON,
						event.GetX(), event.GetY());
#if defined(_MSC_VER)
				ReleaseCapture();
#endif
				Refresh();
			}
		}
		else if (event.GetWheelRotation())
		{
			GetViewport().control().OnMouseWheel(
					0,
					event.GetWheelRotation()
							* event.GetWheelDelta(),
					event.GetX(), event.GetY());
			Refresh();
		}
		else if (event.Dragging())
		{
#if defined(_MSC_VER)
			if ( ::GetCapture() == (HWND)this->GetHWND() )
#else
			if (1)
#endif
			{
				eh::Controller::Flags flags = 0;

				if (event.LeftIsDown())
					flags |= Controller::LBUTTON;

				if (event.RightIsDown())
					flags |= Controller::RBUTTON;

				GetViewport().control().OnMouseMove(flags,
						event.GetX(), event.GetY());

				Refresh();

				//if(m_pViewport->getModeFlag(Camera::MODE_DRAW_NOT_SIMPLE) == false)
				//	SetTimer(0, 300,0);
			}
			else
				GetViewport().control().OnMouseMove(0,
						event.GetX(), event.GetY());

			if (!GetViewport().isValid())
				Refresh();
		}

	}

};
