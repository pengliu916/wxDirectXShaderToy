#pragma once
class DXGfxCore;
class DXFrame : public wxFrame
{
public:
	DXFrame();
	~DXFrame();

	void OnQuit(wxCommandEvent&);
	void OnInit();
	void OnIdle(wxIdleEvent&);

private:
	DXGfxCore* _GfxCore;

	DECLARE_EVENT_TABLE();
};