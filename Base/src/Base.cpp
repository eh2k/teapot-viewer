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

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/sysopt.h>
#include <wx/hyperlink.h>
#include <wx/dnd.h>

#include "Base3DWnd.h"
#include "OpenGLWnd.h"
#if defined(_MSC_VER)
#include "Direct3DWnd.h"
#endif

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <Scene.h>
#include <SceneIO.h>
using namespace eh;

const char* CURRENT_VERSION = "0.9a";
const char* PROJECT_URL = "http://teapot-viewer.googlecode.com";
const char* UPDATECHECK_URL = "http://teapot-viewer.googlecode.com/files/UpdateCheck.0.9a.txt";

class MainFrame: public wxFrame
{
public:
	enum wxID
	{
		wxID_WIREFRAME = wxID_HIGHEST,
		wxID_LIGHTING,
		wxID_SHADOW,
		wxID_BACKGROUND,
		wxID_BOUNDINGS,
		wxID_AABBTREE,
		wxID_FULLSCREEN,
		wxID_CAMERA_RESET,
		wxID_PERSPECTIVE,
		wxID_ORTHOGONAL,
		wxID_CAMERA0,
		wxID_CAMERA1,
		wxID_CAMERA2,
		wxID_CAMERA3,
		wxID_CAMERA4,
		wxID_CAMERA5,
		wxID_CAMERA7,
		wxID_CAMERA8,
		wxID_CAMERA9,
		wxEVT_NEWVERSION
	};

	MainFrame():
		wxFrame(NULL, wxID_ANY, wxT("Teapot Viewer"), wxDefaultPosition, wxSize(640, 480)),
		m_p3DWnd(NULL), m_pGauge(NULL),
		m_pSceneIO(NULL)
	{
		m_pInstance = this;

		this->SetIcon( wxIcon(wxT("BASE_ICO")) );

		wxSystemOptions::SetOption(wxT("msw.remap"), 2);

		class DropTarget: public wxFileDropTarget
        {
            virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
            {
                //TODO: Multiple Files...
				MainFrame::m_pInstance->LoadModel( std::wstring(filenames[0].c_str()), false );
                return true;
            }
        };

        wxWindow::SetDropTarget(new DropTarget());

		//Menu

		wxMenu* pFileMenu = new wxMenu;
		pFileMenu->Append(wxID_OPEN, _T("&Open...\tCtrl-O"));
		Connect( wxID_OPEN, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnOpenFile));

		pFileMenu->Append(wxID_SAVE, _T("&Export..\tCtrl-S"));
		Connect( wxID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSaveFile));

		pFileMenu->AppendSeparator();
		pFileMenu->Append(wxID_EXIT, _T("&Exit\tAlt-F4"));
		Connect( wxID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnExit));

		wxMenu* pViewMenu = new wxMenu;
		pViewMenu->AppendCheckItem(wxID_WIREFRAME, _T("&Wireframe\tW"));
		pViewMenu->AppendCheckItem(wxID_LIGHTING, _T("&Lighting\tL"))->Check();
		pViewMenu->AppendCheckItem(wxID_SHADOW, _T("&Shadow\tS"))->Check();
		pViewMenu->AppendCheckItem(wxID_BACKGROUND, _T("Back&ground\tG"))->Check();
		pViewMenu->AppendSeparator();
		pViewMenu->AppendCheckItem(wxID_BOUNDINGS, _T("&BoundingBoxes\tB"))->Check(false);
		pViewMenu->AppendCheckItem(wxID_AABBTREE, _T("Sce&ne-AABB-Tree\tN"))->Check(false);
		pViewMenu->AppendSeparator();
		Connect( wxID_WIREFRAME, wxID_AABBTREE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnView));

		wxMenu* pCameraMenu = new wxMenu;
		pCameraMenu->AppendRadioItem(wxID_PERSPECTIVE, _T("&Perspective Projection\tP"));
		pCameraMenu->AppendRadioItem(wxID_ORTHOGONAL, _T("&Orthogonal Projection\tO"));
		Connect( wxID_CAMERA_RESET, wxID_CAMERA9, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnCamera));

		pViewMenu->AppendCheckItem(wxID_FULLSCREEN, _T("Fullscreen\tF11"))->Check(false);
		Connect( wxID_FULLSCREEN, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnFullscreen));


		wxMenu* pHelpMenu = new wxMenu;
		pHelpMenu->Append(wxID_ABOUT, _T("&About.."), _T("About.."));

		Connect( wxID_ABOUT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnAbout));

		wxMenuBar* menuBar = new wxMenuBar( wxMB_DOCKABLE );
		menuBar->Append(pFileMenu, _T("&File"));
		menuBar->Append(pViewMenu, _T("&View"));
		menuBar->Append(pCameraMenu, _T("&Camera"));
		menuBar->Append(pHelpMenu, _T("&Help"));
		SetMenuBar(menuBar);

		//Statusbar

		this->CreateStatusBar();

		//////////////////////////

		if( m_p3DWnd == NULL )
		{

#if defined(_MSC_VER)
			m_p3DWnd = new Direct3D9Wnd(this);
#else
			m_p3DWnd = new OpenGLWnd(this);
#endif
		}

		/////////////////////////

        class CheckForNewVersionThread: public wxThread
        {
        public:
            CheckForNewVersionThread() : wxThread(wxTHREAD_DETACHED)
            {
                if(wxTHREAD_NO_ERROR == Create())
                {
                    Run();
                }
            }
        protected:
            virtual ExitCode Entry()
            {
                std::string CheckForUpdate(const char* httpUrl);

                std::string ret = CheckForUpdate(UPDATECHECK_URL);

                wxThreadEvent* te = new wxThreadEvent(wxEVT_THREAD, wxEVT_NEWVERSION);
                te->SetPayload(ret);
                wxQueueEvent(MainFrame::m_pInstance, te);

                return static_cast<ExitCode>(NULL);
            }
        };

        new CheckForNewVersionThread();

	}

	virtual wxStatusBar* CreateStatusBar(int number = 1,
                                         long style = wxSTB_DEFAULT_STYLE,
                                         wxWindowID winid = 0,
                                         const wxString& name = wxStatusLineNameStr)
    {
        wxFrame::CreateStatusBar();

        wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->AddStretchSpacer(1);

		m_pGauge = new wxGauge( this->GetStatusBar(), wxID_ANY, 100,
                                wxDefaultPosition, wxDefaultSize,
                                wxNO_BORDER|wxHORIZONTAL|wxGA_SMOOTH);

#if defined(_MSC_VER)
        sizer->Add(m_pGauge, 1, wxALL|wxEXPAND|wxALIGN_RIGHT, 5);
#else
        sizer->Add(m_pGauge, 1, wxALL|wxEXPAND|wxALIGN_RIGHT, 2);
#endif
        this->GetStatusBar()->SetSizer(sizer);

		this->GetStatusBar()->Layout();
        m_pGauge->Hide();

        return this->GetStatusBar();
    }

	void OnNewVersion(wxThreadEvent& evt)
    {
        std::string updateCheck = evt.GetPayload<std::string>();

        wxSizer* sizer = this->GetStatusBar()->GetSizer();

        size_t a = updateCheck.find('\t');
        size_t b = updateCheck.find('\t', a+1);

		wxHyperlinkCtrl* hyperlink = NULL;

        if(a < b)
        {
            std::string version = updateCheck.substr(0, a );
            std::string text = updateCheck.substr(a+1, b-a );
            std::string url = updateCheck.substr(b+1, updateCheck.size()-b );


            if(version != CURRENT_VERSION)
                hyperlink = new wxHyperlinkCtrl(this->GetStatusBar(), wxID_ANY, wxString(text.c_str()), wxString(url.c_str()));
        }

		if(hyperlink == NULL)
				hyperlink = new wxHyperlinkCtrl(this->GetStatusBar(), wxID_ANY, wxT("Teapot-Viewer Website"), wxString(PROJECT_URL));


#if defined(_MSC_VER)
       sizer->Add(hyperlink, 0, wxALL|wxEXPAND|wxALIGN_RIGHT, 5);
#else
       sizer->Add(hyperlink, 0, wxALL|wxEXPAND|wxALIGN_RIGHT, 0);
#endif


        sizer->AddSpacer(10);
    }

	virtual ~MainFrame()
	{
		if(m_pSceneIO)
			delete m_pSceneIO;
	}

	virtual void OnInternalIdle()
	{
		this->GetStatusBar()->Layout();

		wxFrame::OnInternalIdle();
	}

	void SetStatusText(const std::wstring& text)
	{
		GetStatusBar()->SetStatusText( text );
	}

	SceneIO* getSceneIO()
	{
		SceneIO::setSetStatusTextCallback( boost::bind(&MainFrame::SetStatusText, this, _1) );

	    if(m_pSceneIO == NULL)
			m_pSceneIO = new SceneIO();

		return m_pSceneIO;
	}

	void OnOpenFile(wxCommandEvent& WXUNUSED(event))
	{

		wxFileDialog dlgOpenFile( this, wxT("Open.."), wxEmptyString, wxEmptyString, getSceneIO()->getFileWildcards().c_str());

		if( dlgOpenFile.ShowModal() == wxID_OK )
		{
			this->UpdateWindowUI();

			LoadModel( std::wstring(dlgOpenFile.GetPath().c_str()) );
		}
	}

	void OnSaveFile(wxCommandEvent& WXUNUSED(event))
	{
		wxFileDialog dlgSaveFile( this, wxT("Export.."), wxEmptyString, wxEmptyString, getSceneIO()->getFileWildcards(false).c_str(), wxFD_SAVE );

		if( dlgSaveFile.ShowModal() == wxID_OK)
		{
			getSceneIO()->write( std::wstring(dlgSaveFile.GetPath().c_str()),
					GetViewport()->getScene(),
					boost::bind(&MainFrame::OnProgress, this, _1));
		}
	}

	void OnFullscreen(wxCommandEvent& event )
	{
		this->ShowFullScreen( event.IsChecked() );
	}

	void OnExit(wxCommandEvent& WXUNUSED(event))
	{
		this->Close();
	}

	void OnAbout(wxCommandEvent& WXUNUSED(event))
	{
		wxSize size(400,400);
        wxPoint pos = this->GetScreenRect().GetTopLeft() + this->GetScreenRect().GetSize() / 2 - size /2;
        wxDialog dlg(this, wxID_ANY, wxString::Format(L"About Teapot-Viewer %s", CURRENT_VERSION), pos, size );


#if defined(_MSC_VER)
        wxStaticBitmap* image = new wxStaticBitmap(&dlg, wxID_ANY, this->GetIcon());
#else
        wxStaticBitmap* image = new wxStaticBitmap(&dlg, wxID_ANY, wxPng(APP_ICO));
#endif

        wxStaticText* label = new wxStaticText(&dlg, wxID_ANY, wxString::Format(L"Teapot-Viewer %s", CURRENT_VERSION));
        wxStaticText* label2 = new wxStaticText(&dlg, wxID_ANY, wxT("Copyright (C) 2010 by E.Heidt"));

		std::string glinfo = GetViewport()->getDriver()->getDriverInformation();
		wxStaticText* label3 = new wxStaticText(&dlg, wxID_ANY, wxString(glinfo));

		wxStaticText* label4 = new wxStaticText(&dlg, wxID_ANY, getSceneIO()->getAboutString().c_str());

        wxBoxSizer* h = new wxBoxSizer(wxHORIZONTAL);
        h->Add(image, 0, wxALL, 20);


        wxBoxSizer* v = new wxBoxSizer(wxVERTICAL);
        v->AddSpacer(32);
        v->Add(label, 1, wxALL, 0);
        v->Add(label2, 1, wxALL, 0);

        wxHyperlinkCtrl* hyperlink = new wxHyperlinkCtrl(&dlg, wxID_ANY, wxString(PROJECT_URL), wxString(PROJECT_URL));

        v->Add(hyperlink, 1, wxALL, 0);
		v->AddStretchSpacer(2);

		v->Add(label4, 1, wxALL, 0);

		v->Add(label3, 1, wxALL, 0);

        h->Add(v, 0, wxALL, 0);

        wxButton* btnOK = new wxButton(&dlg, wxID_OK, wxT("OK"));
        h->Add(btnOK, 1, wxALL, 20);


        dlg.SetSizer(h);

        dlg.ShowModal();

		//std::string glinfo = GetViewport()->getDriver()->getDriverInformation();

		//std::wstring about;
		//about += L"TeapotViewer 1.0\n\n";
		//about += L"Copyright (C) 2007-2010 by E.Heidt\nhttp://teapot-viewer.sourceforge.net\n\n";
		//about += getSceneIO()->getAboutString();
		//about += L"\n\n";
		//about += std::wstring(glinfo.begin(), glinfo.end());
		//wxMessageBox( about.c_str() );
	}

	bool LoadModel(const std::wstring& sFile, bool bAsThread = false )
	{
		m_sCurrentFile = sFile.c_str();

		Ptr<Scene> pScene = Scene::create();

		bool ret = getSceneIO()->read( sFile, pScene, boost::bind(&MainFrame::OnProgress, this, _1) );

		wxMenu* pCameraMenu = new wxMenu;
		pCameraMenu->AppendRadioItem(wxID_PERSPECTIVE, _T("&Perspective Projection\tP"));
		pCameraMenu->AppendRadioItem(wxID_ORTHOGONAL, _T("&Orthogonal Projection\tO"));
		pCameraMenu->AppendSeparator();
		pCameraMenu->Append(wxID_CAMERA_RESET, _T("&Default\t1"));

		for(size_t i = 0; i < pScene->getCameras().size(); i++)
		{
			Ptr<Camera> pCam = pScene->getCameras()[i];
			pCameraMenu->Append(wxID_CAMERA0+i, wxString::From8BitData(pCam->getName().c_str()) + wxString::Format(wxT("\t%d"),i+2) );
		}

		wxMenu* old = GetMenuBar()->Replace(2, pCameraMenu, _T("&Camera"));
		delete old;

		GetViewport()->setScene(pScene);
		ResetView();

		this->SetTitle( SceneIO::File(sFile).getName() );
		return ret;
	}

protected:

	void ResetView()
	{
		if(OpenGLWnd* wnd = dynamic_cast<OpenGLWnd*>(m_p3DWnd))
			return wnd->ResetView();
#if defined(_MSC_VER)
		if(Direct3D9Wnd* wnd = dynamic_cast<Direct3D9Wnd*>(m_p3DWnd))
			return wnd->ResetView();
#endif
	}

	Viewport* GetViewport()
	{
		if(OpenGLWnd* wnd = dynamic_cast<OpenGLWnd*>(m_p3DWnd))
			return &wnd->GetViewport();
#if defined(_MSC_VER)
		if(Direct3D9Wnd* wnd = dynamic_cast<Direct3D9Wnd*>(m_p3DWnd))
			return &wnd->GetViewport();
#endif
		return NULL;
	}

	void OnCamera(wxCommandEvent& event)
	{
		switch( event.GetId() )
		{
		case wxID_CAMERA_RESET:
			GetViewport()->setScene( GetViewport()->getScene(), GetViewport()->getScene()->createOrbitalCamera() );
			ResetView();
			break;
		case wxID_PERSPECTIVE:
		case wxID_ORTHOGONAL:
			GetViewport()->setModeFlag( Viewport::MODE_ORTHO, event.GetId() != wxID_PERSPECTIVE );
			this->GetMenuBar()->Check( wxID_PERSPECTIVE, event.GetId() == wxID_PERSPECTIVE );
			this->GetMenuBar()->Check( wxID_ORTHOGONAL, event.GetId() == wxID_ORTHOGONAL );
			break;
		default:
			{
			Uint iCamera = event.GetId() - wxID_CAMERA0;
			GetViewport()->setScene( GetViewport()->getScene(), GetViewport()->getScene()->getCameras()[iCamera] );
			}
			break;
		}
		m_p3DWnd->Refresh();
	}
	void OnView(wxCommandEvent& event)
	{
		switch( event.GetId() )
		{
		case wxID_WIREFRAME:
			GetViewport()->setModeFlag( Viewport::MODE_WIREFRAME, event.IsChecked() );
			break;
		case wxID_LIGHTING:
			GetViewport()->setModeFlag( Viewport::MODE_LIGHTING, event.IsChecked() );
			break;
		case wxID_SHADOW:
			GetViewport()->setModeFlag( Viewport::MODE_SHADOW, event.IsChecked() );
			break;
		case wxID_BACKGROUND:
			GetViewport()->setModeFlag( Viewport::MODE_BACKGROUND, event.IsChecked() );
			break;
		case wxID_BOUNDINGS:
			GetViewport()->setModeFlag( Viewport::MODE_DRAWPRIMBOUNDS, event.IsChecked() );
			break;
		case wxID_AABBTREE:
			GetViewport()->setModeFlag( Viewport::MODE_DRAWAABBTREE, event.IsChecked() );
			break;
		}

		m_p3DWnd->Refresh();
	}

	void OnProgress(float p)
	{
		if(p == 1.f)
		{
			m_pGauge->Hide();
		}
		else
		{
			m_pGauge->SetValue( (int)(p*100.f));
            m_pGauge->Show();
			this->GetStatusBar()->Layout();

			SetStatusText( std::wstring(wxString::Format(wxT("Processing %s ... %d%%"),
							SceneIO::File(m_sCurrentFile).getName().c_str(), (int)(p*100) ).c_str()) );
		}
	}

	wxWindow* m_p3DWnd;

	wxGauge* m_pGauge;
	SceneIO* m_pSceneIO;
	std::wstring m_sCurrentFile;

	static MainFrame* m_pInstance;

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_THREAD(wxEVT_NEWVERSION, MainFrame::OnNewVersion)
wxEND_EVENT_TABLE()

MainFrame* MainFrame::m_pInstance = NULL;

class MyApp : public wxApp
{
public:
	virtual bool OnInit()
	{
		MainFrame* pFrame = new MainFrame();

		pFrame->Show(true);
#if defined(_MSC_VER)
		for(int i = 1; i < this->argc; i++)
#else
		for(int i = 0; i < this->argc; i++)
#endif
		{
			if(!boost::equals(this->argv[i], "/direct3d9"))
			if(!boost::equals(this->argv[i], "/opengl"))
				pFrame->LoadModel( std::wstring(this->argv[i]) );
		}

		return true;
	}
};

IMPLEMENT_APP(MyApp)
//IMPLEMENT_APP_CONSOLE(MyApp);

