//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of main frame for displaying game
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <fstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "gambit.h"
#include "game-frame.h"

#include "dialog-about.h"        // for "About..." dialog

#include "table-schelling.h"
#include "panel-nash.h"

const int GBT_MENU_TOOLS_EQM = 1000;

BEGIN_EVENT_TABLE(gbtGameFrame, wxFrame)
  EVT_MENU(wxID_NEW, gbtGameFrame::OnFileNew)
  EVT_MENU(wxID_OPEN, gbtGameFrame::OnFileOpen)
  EVT_MENU(wxID_CLOSE, gbtGameFrame::OnFileClose)
  EVT_MENU(wxID_SAVE, gbtGameFrame::OnFileSave)
  EVT_MENU(wxID_EXIT, gbtGameFrame::OnFileExit)
  EVT_MENU(GBT_MENU_TOOLS_EQM, gbtGameFrame::OnToolsEquilibrium)
  EVT_MENU(wxID_ABOUT, gbtGameFrame::OnHelpAbout)
END_EVENT_TABLE()

//-------------------------------------------------------------------------
//                       gbtGameFrame: Lifecycle
//-------------------------------------------------------------------------

gbtGameFrame::gbtGameFrame(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxFrame(p_parent, -1, "",
	    wxDefaultPosition, wxSize(800, 600)), 
    gbtGameView(p_doc)
{
  MakeMenu();

  m_tablePanel = new gbtTableSchelling(this, p_doc);
  m_algorithmPanel = new gbtNashPanel(this, p_doc);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(m_tablePanel, 1, wxEXPAND, 0);
  sizer->Add(m_algorithmPanel, 1, wxEXPAND, 0);
  sizer->Show(m_algorithmPanel, false);
  SetSizer(sizer);
  Layout();

  Show(true);
}

//-------------------------------------------------------------------------
//          gbtGameFrame: Auxiliary functions for setting up frame
//-------------------------------------------------------------------------

void gbtGameFrame::MakeMenu(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_NEW, _("&New"), _("Create a new game"));
  fileMenu->Append(wxID_OPEN, _("&Open"), _("Open a saved game"));
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_SAVE, _("&Save"), _("Save this game"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, _("E&xit"), _("Exit Gambit"));

  wxMenu *toolsMenu = new wxMenu;	
  toolsMenu->Append(GBT_MENU_TOOLS_EQM, _("&Equilibrium"),
		    _("Compute Nash equilibria of the game"), true);
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, _("&About"), _("About Gambit"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(toolsMenu, _("&Tools"));
  menuBar->Append(helpMenu, _("&Help"));

  SetMenuBar(menuBar);
}

//-------------------------------------------------------------------------
//                 gbtGameFrame: Menu command handlers
//-------------------------------------------------------------------------

void gbtGameFrame::OnFileNew(wxCommandEvent &)
{
  gbtArray<int> dim(2);  dim[1] = dim[2] = 2;
  (void) new gbtGameFrame(0, new gbtGameDocument(NewNfg(dim)));
}


void gbtGameFrame::OnFileOpen(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), wxT(""), wxT(""), 
		      _("Extensive form games (*.efg)|*.efg|"
			"Normal form games (*.nfg)|*.nfg|"
			"All files|*.*"));

  if (dialog.ShowModal() == wxID_OK) {
    std::ifstream file(dialog.GetPath().mb_str());
    try {
      if (file.is_open()) {
	gbtGame nfg = ReadNfg(file);
	if (file.bad()) {
	  wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
					(const char *) dialog.GetPath().mb_str()),
		       _("Error"), wxOK, 0);
	}
	else {
	  (void) new gbtGameFrame(0, new gbtGameDocument(nfg));
	}
      }
      else {
	wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				      (const char *) dialog.GetPath().mb_str()),
		     _("Error"), wxOK, 0);
      }
    }
    catch (gbtNfgParserException &) {
      wxMessageBox(wxString::Format(_("File '%s' not in a recognized format"),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, 0);

    }
  }
}

void gbtGameFrame::OnFileClose(wxCommandEvent &)
{
  Close();
}

void gbtGameFrame::OnFileSave(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"),
		      //		      wxPathOnly(m_doc->GetFilename()),
		      _T(""),
		      //		      wxFileNameFromPath(m_doc->GetFilename()), 
		      _T(""),
		      wxT("Normal form files (*.nfg)|*.nfg"),
		      wxSAVE | wxOVERWRITE_PROMPT);

  switch (dialog.ShowModal()) {
  case wxID_OK:
    // Save the filename in the document...
    break;
  case wxID_CANCEL:
  default:
    return;
  }

  try {
    std::ofstream file(dialog.GetPath().mb_str());
    if (file.is_open()) {
      m_doc->GetGame()->WriteNfg(file);
      if (file.bad()) {
	wxMessageBox(wxString::Format(_("Write error occurred in saving %s."),
				      (const char *) dialog.GetPath().mb_str()),
		     _("Error"), wxOK, this);
      }      
    }
    else {
      wxMessageBox(wxString::Format(_("Could not open %s for writing."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
  catch (gbtException &) {
    wxMessageBox(_("An internal error occurred in Gambit"), 
		 _("Error"), wxOK, this);
  }
}

void gbtGameFrame::OnFileExit(wxCommandEvent &)
{
  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void gbtGameFrame::OnToolsEquilibrium(wxCommandEvent &)
{
  GetSizer()->Show(m_algorithmPanel,
		   !GetSizer()->IsShown(m_algorithmPanel));
  Layout();
}

void gbtGameFrame::OnHelpAbout(wxCommandEvent &)
{
  gbtAboutDialog(this).ShowModal();
}


//-------------------------------------------------------------------------
//           gbtGameFrame: Implementation of view interface
//-------------------------------------------------------------------------

void gbtGameFrame::OnUpdate(void)
{

}
