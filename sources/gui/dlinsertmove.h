//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog to insert move into a tree
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

#ifndef DLINSERTMOVE_H
#define DLINSERTMOVE_H

#include "wx/spinctrl.h"

class dialogInsertMove : public wxDialog {
private:
  gbtEfgGame &m_efg;

  wxListBox *m_playerItem, *m_infosetItem;
  wxSpinCtrl *m_actions;

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);

public:
  // Constructor
  dialogInsertMove(wxWindow *, gbtEfgGame &);

  // Data access (only valid if ShowModal() returns wxID_OK.
  // If GetInfoset() returns null, user selected "new infoset"
  gbtEfgPlayer GetPlayer(void) const;
  gbtEfgInfoset GetInfoset(void) const;
  int GetActions(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLMOVEADD_H



