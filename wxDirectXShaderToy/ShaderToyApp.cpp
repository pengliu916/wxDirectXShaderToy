#include <wx/wx.h>
#include "ShaderToyApp.h"

IMPLEMENT_APP(ShaderToyApp)

bool ShaderToyApp::OnInit()
{
	wxFrame *frame = new wxFrame((wxFrame*)NULL, -1, _T("Hello World"));
	frame->CreateStatusBar();
	frame->SetStatusText(_T("Hello World"));
	frame->Show(true);
	SetTopWindow(frame);
	return true;
}