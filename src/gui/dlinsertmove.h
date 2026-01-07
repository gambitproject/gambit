//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

#ifndef GAMBIT_GUI_DLINSERTMOVE_H
#define GAMBIT_GUI_DLINSERTMOVE_H

#include <wx/spinctrl.h>
#include "gamedoc.h"

namespace Gambit::GUI {

class InsertMoveDialog final : public wxDialog {
  GameDocument *m_doc;

  wxChoice *m_playerItem, *m_infosetItem;
  wxSpinCtrl *m_actions;

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);

public:
  // Constructor
  InsertMoveDialog(wxWindow *, GameDocument *);

  // Data access (only valid if ShowModal() returns wxID_OK.
  // If GetInfoset() returns null, user selected "new infoset"
  GamePlayer GetPlayer() const;
  GameInfoset GetInfoset() const;
  int GetActions() const;
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLINSERTMOVE_H
