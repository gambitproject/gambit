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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "gambit.h"
#include "game-frame.h"

#include "dialog-about.h"        // for "About..." dialog

BEGIN_EVENT_TABLE(gbtGameFrame, wxFrame)
  EVT_MENU(wxID_EXIT, gbtGameFrame::OnFileExit)
  EVT_MENU(wxID_ABOUT, gbtGameFrame::OnHelpAbout)
END_EVENT_TABLE()

//-------------------------------------------------------------------------
//                       gbtGameFrame: Lifecycle
//-------------------------------------------------------------------------

gbtGameFrame::gbtGameFrame(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxFrame(p_parent, -1, ""), gbtGameView(p_doc)
{
  MakeMenu();
  Show(true);
}

//-------------------------------------------------------------------------
//          gbtGameFrame: Auxiliary functions for setting up frame
//-------------------------------------------------------------------------

void gbtGameFrame::MakeMenu(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_EXIT, _("E&xit"), _("Exit Gambit"));

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, _("&About"), _("About Gambit"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(helpMenu, _("&Help"));

  SetMenuBar(menuBar);
}

//-------------------------------------------------------------------------
//                 gbtGameFrame: Menu command handlers
//-------------------------------------------------------------------------

void gbtGameFrame::OnFileExit(wxCommandEvent &)
{
  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
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
