//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dleditmove.h
// Dialog for viewing and editing properties of a move
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

#ifndef GAMBIT_GUI_DLEDITMOVE_H
#define GAMBIT_GUI_DLEDITMOVE_H

namespace Gambit::GUI {
class ActionSheet;

class EditMoveDialog final : public wxDialog {
  GameInfoset m_infoset;
  wxChoice *m_player;
  wxTextCtrl *m_infosetName;
  ActionSheet *m_actionSheet;

  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  EditMoveDialog(wxWindow *p_parent, const GameInfoset &p_infoset);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetInfosetName() const { return m_infosetName->GetValue(); }
  int GetPlayer() const { return (m_player->GetSelection() + 1); }

  int NumActions() const;
  wxString GetActionName(int p_act) const;
  Array<Number> GetActionProbs() const;

  wxDECLARE_EVENT_TABLE();
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLEDITMOVE_H
