//
// FILE: wxgcl.cc -- Experimental wxWindows front-end for GCL
//
// $Id$
//

#include "wx/wx.h"
#include "wx/splitter.h"

#include "gsm.h"

class GclApp : public wxApp {
private:
  bool OnInit(void);

public:
  virtual ~GclApp() { }
};

DECLARE_APP(GclApp)

IMPLEMENT_APP(GclApp)

class GclFrame : public wxFrame {
private:
  wxTextCtrl *m_outputWindow, *m_inputWindow;

  // Menu event handlers
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);

public:
  GclFrame(wxFrame *p_parent, const wxString &p_title,
	   const wxPoint &p_position, const wxSize &p_size);
  virtual ~GclFrame();

  DECLARE_EVENT_TABLE()
};

bool GclApp::OnInit(void)
{
  GclFrame *gclFrame = new GclFrame(0, "Gammbit Command Language",
				    wxPoint(0, 0), wxSize(400, 400));
  return true;
}

GclFrame::GclFrame(wxFrame *p_parent, const wxString &p_title,
		   const wxPoint &p_position, const wxSize &p_size)
  : wxFrame(p_parent, -1, p_title, p_position, p_size)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_EXIT, "&Quit", "Quit program");
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, "&About", "About this program");
  helpMenu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(helpMenu, "&Help");
  SetMenuBar(menuBar);

  CreateStatusBar();
  /*
  wxSplitterWindow *splitter = new wxSplitterWindow(this);
  m_outputWindow = new wxTextCtrl(splitter, -1, "",
				  wxDefaultPosition, wxDefaultSize,
				  wxTE_MULTILINE | wxTE_READONLY);
  m_inputWindow = new wxTextCtrl(splitter, -1, "",
				 wxDefaultPosition, wxDefaultSize,
				 wxTE_MULTILINE);
  splitter->SplitVertically(m_outputWindow, m_inputWindow);
  */
  Show(true);
}

GclFrame::~GclFrame()
{ }

BEGIN_EVENT_TABLE(GclFrame, wxFrame)
  EVT_MENU(wxID_EXIT, wxWindow::Close)
  EVT_MENU(wxID_ABOUT, GclFrame::OnHelpAbout)
  EVT_MENU(wxID_HELP_CONTENTS, GclFrame::OnHelpContents)
  EVT_CLOSE(GclFrame::OnCloseWindow)
END_EVENT_TABLE()

void GclFrame::OnHelpAbout(wxCommandEvent &)
{ }

void GclFrame::OnHelpContents(wxCommandEvent &)
{ }

void GclFrame::OnCloseWindow(wxCloseEvent &)
{
  Destroy();
}

GSM* _gsm;
char* _SourceDir = NULL;
char* _ExePath = NULL;
/*
gFileInput _gin(stdin);
gInput &gin = _gin;
*/

