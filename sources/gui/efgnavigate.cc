//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of extensive form navigation window
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgnavigate.h"

EfgNavigateWindow::EfgNavigateWindow(gbtGameDocument *p_doc,
				     wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_doc(p_doc)
{
  CreateGrid(10, 1);
  SetEditable(false);
  SetDefaultCellAlignment(wxCENTER, wxCENTER);

  SetLabelValue(wxVERTICAL, "Node label", 0);
  SetLabelValue(wxVERTICAL, "Pr(node reached)", 1);
  SetLabelValue(wxVERTICAL, "Node value", 2);
  SetLabelValue(wxVERTICAL, "Information set", 3);
  SetLabelValue(wxVERTICAL, "Pr(infoset reached)", 4);
  SetLabelValue(wxVERTICAL, "Belief", 5);
  SetLabelValue(wxVERTICAL, "Information set value", 6);
  SetLabelValue(wxVERTICAL, "Incoming action label", 7);
  SetLabelValue(wxVERTICAL, "Pr(incoming action)", 8);
  SetLabelValue(wxVERTICAL, "Incoming action value", 9);

  SetLabelSize(wxHORIZONTAL, 0);
  SetLabelSize(wxVERTICAL, 150);

  EnableGridLines(false);
  for (int row = 0; row < 10; row++) {
    if (row % 2 == 0) {
      SetCellBackgroundColour(row, 0, wxColour(200, 200, 200));
    }
    else {
      SetCellBackgroundColour(row, 0, wxColour(225, 225, 225));
    }
  }

  AdjustScrollbars();
  Show(true);
}

void EfgNavigateWindow::OnUpdate(void)
{
  SetDefaultCellFont(m_doc->GetPreferences().GetDataFont());
  SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  SetLabelFont(m_doc->GetPreferences().GetLabelFont());

  gbtEfgNode cursor = m_doc->GetCursor();
  
  if (cursor.IsNull()) { // no data available
    for (int i = 0; i < GetRows(); i++) { 
      SetCellValue("", i, 0);
    }
    return;
  }

  // if we got here, the node is valid.
  try {
    SetCellValue((char *) cursor.GetLabel(), 0, 0);
    SetCellValue((char *) m_doc->GetRealizProb(cursor), 1, 0);
    SetCellValue((char *) m_doc->GetNodeValue(cursor), 2, 0);

    gText tmpstr;
  
    if (cursor.GetPlayer().IsNull()) {
      tmpstr = "TERMINAL";
    }
    else {
      if (cursor.GetPlayer().IsChance())
	tmpstr = "CHANCE";
      else
	tmpstr = ("(" + ToText(cursor.GetPlayer().GetId()) + "," +
		  ToText(cursor.GetInfoset().GetId()) + ")");
    }
	  
    SetCellValue((char *) tmpstr, 3, 0);
    SetCellValue((char *) m_doc->GetInfosetProb(cursor), 4, 0);
    SetCellValue((char *) m_doc->GetBeliefProb(cursor), 5, 0);
    SetCellValue((char *) m_doc->GetInfosetValue(cursor), 6, 0);
	
    gbtEfgNode p = cursor.GetParent();

    if (!p.IsNull()) {
      int branch = 0;
      for (branch = 1; p.GetChild(branch) != cursor; branch++);

      SetCellValue((char *) cursor.GetPriorAction().GetLabel(), 7, 0);
      SetCellValue((char *) m_doc->GetActionProb(p, branch), 8, 0);
      SetCellValue((char *) m_doc->GetActionValue(p, branch), 9, 0);
    }
    else {
      SetCellValue("N/A (root)", 7, 0);
      SetCellValue("1", 8, 0);
      SetCellValue("N/A", 9, 0);
    }
  }	
  catch (gException &) { }
}

//-------------------------------------------------------------------------
//                      class gbtEfgNavigateFrame
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtEfgNavigateFrame, wxFrame)
  EVT_CLOSE(gbtEfgNavigateFrame::OnClose)
  EVT_MENU(wxID_CLOSE, gbtEfgNavigateFrame::Close)
END_EVENT_TABLE()

gbtEfgNavigateFrame::gbtEfgNavigateFrame(gbtGameDocument *p_doc,
					 wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(300, 200)),
    gbtGameView(p_doc)
{
  m_grid = new EfgNavigateWindow(p_doc, this);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");

  wxMenu *editMenu = new wxMenu;

  wxMenu *viewMenu = new wxMenu;

  wxMenu *formatMenu = new wxMenu;

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(formatMenu, "&Format");
  SetMenuBar(menuBar);

  Show(false);
}

gbtEfgNavigateFrame::~gbtEfgNavigateFrame()
{ }

//--------------------------------------------------------------------------
//                  gbtEfgNavigateFrame: Event handlers
//--------------------------------------------------------------------------

void gbtEfgNavigateFrame::OnClose(wxCloseEvent &p_event)
{
  m_doc->SetShowEfgNavigate(false);
  // Frame is now hidden; leave it that way, don't actually close
  p_event.Veto();
}

void gbtEfgNavigateFrame::OnUpdate(gbtGameView *p_sender)
{
  if (m_doc->ShowEfgNavigate()) {
    m_grid->OnUpdate();
    wxSize size = m_grid->GetBestSize();
    SetClientSize(size);
    m_grid->SetSize(size.GetWidth() + 1, size.GetHeight() + 1);
    m_grid->SetScrollRate(0, 0);

    if (m_doc->GetFilename() != "") {
      SetTitle(wxString::Format("Gambit - Node View: [%s] %s", 
				m_doc->GetFilename().c_str(), 
				(char *) m_doc->GetNfg().GetLabel()));
    }
    else {
      SetTitle(wxString::Format("Gambit - Node View: %s",
				(char *) m_doc->GetNfg().GetLabel()));
    }
  }
  Show(m_doc->ShowEfgNavigate());
}


