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
#include "table-matrix.h"
#include "panel-nash.h"

const int GBT_MENU_TOOLS_EQM = 1000;

BEGIN_EVENT_TABLE(gbtGameFrame, wxFrame)
  EVT_MENU(wxID_NEW, gbtGameFrame::OnFileNew)
  EVT_MENU(wxID_OPEN, gbtGameFrame::OnFileOpen)
  EVT_MENU(wxID_CLOSE, gbtGameFrame::OnFileClose)
  EVT_MENU(wxID_SAVE, gbtGameFrame::OnFileSave)
  EVT_MENU(wxID_EXIT, gbtGameFrame::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, gbtGameFrame::OnFileMRU)
  EVT_MENU(GBT_MENU_TOOLS_EQM, gbtGameFrame::OnToolsEquilibrium)
  EVT_MENU(wxID_ABOUT, gbtGameFrame::OnHelpAbout)
  EVT_CLOSE(gbtGameFrame::OnCloseWindow)
END_EVENT_TABLE()

//-------------------------------------------------------------------------
//                       gbtGameFrame: Lifecycle
//-------------------------------------------------------------------------

gbtGameFrame::gbtGameFrame(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxFrame(p_parent, -1, "",
	    wxDefaultPosition, wxSize(800, 600)), 
    gbtGameView(p_doc)
{
  wxGetApp().AddWindow(this);
  MakeMenu();

  if (p_doc->GetGame()->NumPlayers() == 2) {
    m_tablePanel = new gbtTableSchelling(this, p_doc);
  }
  else {
    m_tablePanel = new gbtTableMatrix(this, p_doc);
  }

  m_algorithmPanel = new gbtNashPanel(this, p_doc);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(m_tablePanel, 1, wxEXPAND, 0);
  sizer->Add(m_algorithmPanel, 1, wxEXPAND, 0);
  sizer->Show(m_algorithmPanel, false);
  SetSizer(sizer);
  Layout();

  OnUpdate();
  Show(true);
}

gbtGameFrame::~gbtGameFrame()
{
  wxGetApp().GetFileHistory()->RemoveMenu(GetMenuBar()->GetMenu(0));
  wxGetApp().RemoveWindow(this);
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

  wxGetApp().GetFileHistory()->UseMenu(fileMenu);
  wxGetApp().GetFileHistory()->AddFilesToMenu(fileMenu);

  SetMenuBar(menuBar);
}

void gbtGameFrame::OnCloseWindow(wxCloseEvent &p_event)
{
  if (p_event.CanVeto() && m_doc->IsModified()) {
    if (wxMessageBox("Game has been modified.  Close anyway?", "Warning",
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }
  p_event.Skip();
}

//-------------------------------------------------------------------------
//                 gbtGameFrame: Menu command handlers
//-------------------------------------------------------------------------

void gbtGameFrame::OnFileNew(wxCommandEvent &)
{
  gbtArray<int> dim(2);  dim[1] = dim[2] = 2;
  gbtGame nfg = NewNfg(dim);
  nfg->SetLabel("Untitled normal form game");
  (void) new gbtGameFrame(0, new gbtGameDocument(nfg));
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
	  gbtGameDocument *doc = new gbtGameDocument(nfg);
	  doc->SetFilename(dialog.GetPath());
	  wxGetApp().GetFileHistory()->AddFileToHistory(dialog.GetPath());
	  (void) new gbtGameFrame(0, doc);
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

void gbtGameFrame::OnFileMRU(wxCommandEvent &p_event)
{
  wxString filename(wxGetApp().GetFileHistory()->GetHistoryFile(p_event.GetId() - wxID_FILE1));

  std::ifstream file(filename.mb_str());
  try {
    if (file.is_open()) {
      gbtGame nfg = ReadNfg(file);
      if (file.bad()) {
	wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				      (const char *) filename.mb_str()),
		     _("Error"), wxOK, 0);
      }
      else {
	gbtGameDocument *doc = new gbtGameDocument(nfg);
	doc->SetFilename(filename);
	wxGetApp().GetFileHistory()->AddFileToHistory(filename);
	(void) new gbtGameFrame(0, doc);
      }
    }
    else {
      wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				    (const char *) filename.mb_str()),
		   _("Error"), wxOK, 0);
    }
  }
  catch (gbtNfgParserException &) {
    wxMessageBox(wxString::Format(_("File '%s' not in a recognized format"),
				  (const char *) filename.mb_str()),
		 _("Error"), wxOK, 0);
    
  }
}

void gbtGameFrame::OnFileClose(wxCommandEvent &)
{
  Close();
}

void gbtGameFrame::OnFileSave(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"),
		      wxPathOnly(m_doc->GetFilename()),
		      wxFileNameFromPath(m_doc->GetFilename()), 
		      wxT("Normal form files (*.nfg)|*.nfg"),
		      wxSAVE | wxOVERWRITE_PROMPT);

  switch (dialog.ShowModal()) {
  case wxID_OK:
    m_doc->SetFilename(dialog.GetPath());
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
      else {
	m_doc->SetModified(false);
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
  for (int i = 1; i <= wxGetApp().NumWindows(); i++) {
    if (wxGetApp().GetWindow(i)->GetDocument()->IsModified()) {
      if (wxMessageBox("There are modified games.  Exit anyway?", "Warning",
		       wxOK | wxCANCEL) == wxCANCEL) {
	return;
      }
      else {
	break;
      }
    }
  }

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
  if (m_doc->GetFilename() == "") {
    SetTitle(wxString::Format("Gambit: [<no file>%s] %s",
			      (m_doc->IsModified()) ? "*" : "",
			      m_doc->GetGame()->GetLabel().c_str()));
  }
  else {
    SetTitle(wxString::Format("Gambit: [%s%s] %s",
			      m_doc->GetFilename().c_str(),
			      (m_doc->IsModified()) ? "*" : "",
			      m_doc->GetGame()->GetLabel().c_str()));
  }
}
