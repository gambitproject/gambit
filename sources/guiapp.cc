//
// FILE: guiapp.cc -- Defines GambitApp application class
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include <wx/toolbar.h>

#include "gstream.h"
#include "efg.h"
#include "nfg.h"

#include "guiapp.h"
#include "gameview.h"
#include "efgframe.h"
#include "nfgframe.h"

IMPLEMENT_APP(gambitApp)

bool gambitApp::OnInit(void)
{
  m_frame = new gambitFrame((wxFrame *) NULL, -1, (char *) "Gambit",
			    wxPoint(-1, -1), wxSize(700, 500),
			    wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL);

  wxMenu *fileMenu = new wxMenu;
  wxMenu *fileNewMenu = new wxMenu;
  fileNewMenu->Append(GAMBIT_NEW_EFG, "&Extensive form");
  fileNewMenu->Append(GAMBIT_NEW_NFG, "&Normal form");
  fileMenu->Append(GAMBIT_NEW, "&New", fileNewMenu);
  wxMenu *fileOpenMenu = new wxMenu;
  fileOpenMenu->Append(GAMBIT_OPEN_EFG, "&Extensive form");
  fileOpenMenu->Append(GAMBIT_OPEN_NFG, "&Normal form");
  fileMenu->Append(GAMBIT_OPEN, "&Open", fileOpenMenu);
  fileMenu->Append(GAMBIT_QUIT, "&Exit");
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(GAMBIT_ABOUT, "&About");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(helpMenu, "&Help");

  m_frame->SetMenuBar(menuBar);
  m_frame->CreateStatusBar();
  m_frame->Show(TRUE);

  SetTopWindow(m_frame);

  return TRUE;
}

BEGIN_EVENT_TABLE(gambitFrame, wxMDIParentFrame)
  EVT_MENU(GAMBIT_ABOUT, gambitFrame::OnAbout)
  EVT_MENU(GAMBIT_NEW_EFG, gambitFrame::OnNewEfg)
  EVT_MENU(GAMBIT_NEW_NFG, gambitFrame::OnNewNfg)
  EVT_MENU(GAMBIT_OPEN_EFG, gambitFrame::OnOpenEfg)
  EVT_MENU(GAMBIT_OPEN_NFG, gambitFrame::OnOpenNfg)
  EVT_SIZE(gambitFrame::OnSize)
  EVT_MENU(GAMBIT_QUIT, gambitFrame::OnQuit)
END_EVENT_TABLE()

gambitFrame::gambitFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
			 const wxPoint& pos, const wxSize& size,
			 const long style)
  : wxMDIParentFrame(parent, id, title, pos, size, style)
{
}

void gambitFrame::OnQuit(wxCommandEvent& WXUNUSED(event) )
{
  Close(TRUE);
}

void gambitFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
  wxMessageBox("Gambit -- Graphics User Interface, Version 0.96\n\n"
	       "Developed by Richard D. McKelvey (rdm@hss.caltech.edu)\n"
	       "Main Programmer:  Theodore Turocy (arbiter@nwu.edu)\n"
	       "Front End: Eugene Grayver (egrayver@hss.caltech.edu)\n"
	       "California Institute of Technology, 1996-9.\n"
	       "Funding provided by the National Science Foundation",
	       "About Gambit");
}

void gambitFrame::OnNewEfg(wxCommandEvent &)
{
  FullEfg *efg = new FullEfg;

  int width, height;
  GetClientSize(&width, &height);
  (void) new guiEfgFrame(this, efg, wxPoint(-1, -1), wxSize(width, height));
}

void gambitFrame::OnNewNfg(wxCommandEvent &)
{
  gArray<int> dim(2);
  dim[1] = 2;
  dim[2] = 2;
  Nfg *nfg = new Nfg(dim);
  int width, height;
  GetClientSize(&width, &height);
  (void) new guiNfgFrame(this, nfg, wxPoint(-1, -1), wxSize(width, height));
}

void gambitFrame::OnOpenEfg(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Open extensive form...");
  dialog.SetWildcard("*.efg");
  if (dialog.ShowModal() == wxID_OK) {
    FullEfg *efg = 0;

    try {
      gFileInput infile(dialog.GetPath());
      efg = ReadEfgFile(infile);

      if (!efg) {
	wxMessageBox(dialog.GetPath() + " is not a valid .efg file");
      }
    }
    catch (gFileInput::OpenFailed &) {
      wxMessageBox("Could not open " + dialog.GetPath() + " for reading");
      return;
    }

    if (efg) {
      int width, height;
      GetClientSize(&width, &height);
      (void) new guiEfgFrame(this, efg, wxPoint(-1, -1), wxSize(width, height));
    }
  }
}

void gambitFrame::OnOpenNfg(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Open normal form...");
  dialog.SetWildcard("*.nfg");
  if (dialog.ShowModal() == wxID_OK) {
    Nfg *nfg = 0;

    try {
      gFileInput infile(dialog.GetPath());
      ReadNfgFile(infile, nfg);

      if (!nfg) {
	wxMessageBox(dialog.GetPath() + " is not a valid .nfg file");
      }
    }
    catch (gFileInput::OpenFailed &) {
      wxMessageBox("Could not open " + dialog.GetPath() + " for reading");
      return;
    }

    if (nfg) {
      int width, height;
      GetClientSize(&width, &height);
      (void) new guiNfgFrame(this, nfg, wxPoint(-1, -1), wxSize(width, height));
    }
  }
}

void gambitFrame::OnSize(wxSizeEvent& WXUNUSED(event) )
{
  int w, h;
  GetClientSize(&w, &h);
    
  GetClientWindow()->SetSize(0, 0, w, h);
}


gNullOutput _gout;
gOutput &gout = _gout;
gNullOutput _gerr;
gOutput &gerr = _gerr;
