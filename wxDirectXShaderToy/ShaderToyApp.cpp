#include <wx/wx.h>
#include "ShaderToyApp.h"
#include "DXFrame.h"

IMPLEMENT_APP(ShaderToyApp)

bool ShaderToyApp::OnInit()
{
	DXFrame *frame = new DXFrame();
	frame->Show(true);
	SetTopWindow(frame);
	return true;
}