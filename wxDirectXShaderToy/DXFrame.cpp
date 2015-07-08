#include <wx/wx.h>

#include "DXGfxCore.h"
#include "DXFrame.h"

enum
{
	ID_Quit = 1,
};

DXFrame::DXFrame()
	: wxFrame((wxFrame *)NULL, wxID_ANY, wxT("wxDirectXShaderToy"),
		wxPoint(20, 20), wxSize(640, 480))
{
	wxMenu *menuFile = new wxMenu;

	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit, _T("E&xit"));

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, _T("&Menu"));

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText(_T("Hello World"));
	_GfxCore = new DXGfxCore();
	OnInit();
}

void DXFrame::OnInit()
{
	_GfxCore->Init(GetHWND());
	_GfxCore->CreateDevice();
}

void DXFrame::OnIdle(wxIdleEvent& event)
{
	_GfxCore->Update();
	_GfxCore->Render();
	event.RequestMore();
}

void DXFrame::OnQuit(wxCommandEvent& event)
{
	Close(true);
}

DXFrame::~DXFrame()
{
	_GfxCore->Destory();
	delete _GfxCore;
}

BEGIN_EVENT_TABLE(DXFrame, wxFrame)
	EVT_IDLE(DXFrame::OnIdle)
	EVT_MENU(ID_Quit,DXFrame::OnQuit)
END_EVENT_TABLE()