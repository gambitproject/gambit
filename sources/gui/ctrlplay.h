//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of a control for selecting a player
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#ifndef CTRLPLAY_H
#define CTRLPLAY_H

#include "gamedoc.h"

BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(GBT_ROW_PLAYER_CHANGE, 7777)
  DECLARE_EVENT_TYPE(GBT_COL_PLAYER_CHANGE, 7778)
END_DECLARE_EVENT_TYPES()

#define EVT_ROW_PLAYER_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        GBT_ROW_PLAYER_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

#define EVT_COL_PLAYER_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        GBT_COL_PLAYER_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),


class gbtPlayerLabelCtrl : public wxScrolledWindow, public gbtGameView {
private:
  bool m_isRowPlayer;
  int m_player;
  wxMenu *m_popupMenu;

  // Implementing gbtGameView members
  void OnUpdate(void) { }

  void OnDraw(wxDC &);

  void OnRightDown(wxMouseEvent &);
  void OnEnterWindow(wxMouseEvent &);
  void OnLeaveWindow(wxMouseEvent &);
  void OnMenu(wxCommandEvent &);

public:
  gbtPlayerLabelCtrl(wxWindow *p_parent, gbtGameDocument *p_doc,
		     bool p_isRowPlayer);
  virtual ~gbtPlayerLabelCtrl();

  void SetPlayer(int p_player);
  int GetPlayer(void) const { return m_player; }

  bool IsRowPlayer(void) const { return m_isRowPlayer; }

  DECLARE_EVENT_TABLE()
};

#endif  // CONTROL_PLAYER_H
