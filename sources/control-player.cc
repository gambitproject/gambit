//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of a control for selecting a player
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

#include "control-player.h"
#include "table-matrix.h"    // Temporary

DEFINE_EVENT_TYPE(GBT_ROW_PLAYER_CHANGE)
DEFINE_EVENT_TYPE(GBT_COL_PLAYER_CHANGE)

//-----------------------------------------------------------------------
//                  class gbtPlayerLabelCtrl
//-----------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtPlayerLabelCtrl, wxScrolledWindow)
  EVT_RIGHT_DOWN(gbtPlayerLabelCtrl::OnRightDown)
  EVT_MENU_RANGE(1000, 32767, gbtPlayerLabelCtrl::OnMenu)
  EVT_ENTER_WINDOW(gbtPlayerLabelCtrl::OnEnterWindow)
  EVT_LEAVE_WINDOW(gbtPlayerLabelCtrl::OnLeaveWindow)
END_EVENT_TABLE()

gbtPlayerLabelCtrl::gbtPlayerLabelCtrl(wxWindow *p_parent,
				       gbtGameDocument *p_doc,
				       bool p_isRowPlayer)
  : wxScrolledWindow(p_parent, -1), gbtGameView(p_doc),
    m_isRowPlayer(p_isRowPlayer), m_player(1),
    m_popupMenu(0)
{
  SetBackgroundColour(wxColour(250, 250, 250));
  if (p_isRowPlayer) {
    SetToolTip(_("The player choosing the row"));
  }
  else {
    SetToolTip(_("The player choosing the column"));
  }
}

gbtPlayerLabelCtrl::~gbtPlayerLabelCtrl()
{
  if (m_popupMenu)  delete m_popupMenu;
}

void gbtPlayerLabelCtrl::SetPlayer(int p_player)
{
  m_player = p_player;
  Refresh();
}

void gbtPlayerLabelCtrl::OnDraw(wxDC &p_dc)
{
  p_dc.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
  wxCoord width, height;

  wxString label = m_doc->GetGame()->GetPlayer(m_player)->GetLabel().c_str();
  wxColour color = m_doc->GetPlayerColor(m_player);
  
  p_dc.GetTextExtent(label, &width, &height);
  p_dc.SetTextForeground(color);
  wxSize size = GetClientSize();

  if (m_isRowPlayer) {
    p_dc.DrawRotatedText(label, 
			 (size.GetWidth() - height) / 2,
			 (size.GetHeight() + width) / 2,
			 90);
  }
  else {
    p_dc.DrawText(label, 
		  (size.GetWidth() - width) / 2,
		  (size.GetHeight() - height) / 2);
  }
}

void gbtPlayerLabelCtrl::OnRightDown(wxMouseEvent &p_event)
{
  if (m_popupMenu)  delete m_popupMenu;

  m_popupMenu = new wxMenu("Show player on this axis");
  const gbtGame &game = m_doc->GetGame();
  for (int pl = 1; pl <= game->NumPlayers(); pl++) {
    m_popupMenu->Append(1000 + pl,
			wxString::Format("%d: %s",
					 pl, 
					 game->GetPlayer(pl)->GetLabel().c_str()));
  }

  PopupMenu(m_popupMenu, p_event.GetX(), p_event.GetY());
}

void gbtPlayerLabelCtrl::OnMenu(wxCommandEvent &p_event)
{
  int newPlayer = p_event.GetId() - 1000;

  if (m_isRowPlayer) {
    wxCommandEvent event(GBT_ROW_PLAYER_CHANGE);
    event.SetInt(newPlayer);
    wxPostEvent(this, event);
  }
  else {
    wxCommandEvent event(GBT_COL_PLAYER_CHANGE);
    event.SetInt(newPlayer);
    wxPostEvent(this, event);
  }
}

void gbtPlayerLabelCtrl::OnEnterWindow(wxMouseEvent &)
{
  SetBackgroundColour(wxColour(210, 210, 210));
  Refresh();
}

void gbtPlayerLabelCtrl::OnLeaveWindow(wxMouseEvent &)
{
  SetBackgroundColour(wxColour(250, 250, 250));
  Refresh();
}

