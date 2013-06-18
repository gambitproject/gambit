//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlinsertmove.h
// Declaration of dialog to insert move into a tree
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

#include <wx/spinctrl.h>
#include "gamedoc.h"

class gbtInsertMoveDialog : public wxDialog {
private:
  gbtGameDocument *m_doc;

  wxChoice *m_playerItem, *m_infosetItem;
  wxSpinCtrl *m_actions;

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);

public:
  // Constructor
  gbtInsertMoveDialog(wxWindow *, gbtGameDocument *);

  // Data access (only valid if ShowModal() returns wxID_OK.
  // If GetInfoset() returns null, user selected "new infoset"
  Gambit::GamePlayer GetPlayer(void) const;
  Gambit::GameInfoset GetInfoset(void) const;
  int GetActions(void) const;
};

#endif  // DLINSERTMOVE_H



