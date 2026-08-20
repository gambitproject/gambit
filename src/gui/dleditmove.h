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

#include "editlabel.h"

namespace Gambit::GUI {
class ActionPanel;

class EditMoveDialog final : public wxDialog {
  GameInfoset m_infoset;
  wxChoice *m_player;
  LabelTextCtrl *m_infosetLabel;
  wxColour m_infosetLabelDefaultBg;
  ActionPanel *m_actionPanel;
  wxStaticText *m_errorText;

  void UpdateValidation();
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  EditMoveDialog(wxWindow *p_parent, const GameInfoset &p_infoset);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetInfosetLabel() const { return m_infosetLabel->GetNormalizedValue(); }
  int GetPlayer() const { return (m_player->GetSelection() + 1); }

  // The number of action rows currently in the panel, after any adds/deletes/reorders.
  // Includes rows marked deleted; check IsDeleted() before committing one.
  int NumActions() const;
  // True if the row at position `p_index` is marked for deletion (kept in the panel, shown
  // disabled, but excluded from the operation this dialog commits).
  bool IsDeleted(int p_index) const;
  // The label the action at position `p_index` had (or the placeholder it was assigned,
  // if newly added) before any edits made in this dialog.  Identifies the action for the
  // structural part of the operation, independent of anything typed into its label field.
  std::string GetStableLabel(int p_index) const;
  // The label as currently typed into the row at position `p_index`.
  wxString GetActionLabel(int p_index) const;
  // The probability currently typed into the row at position `p_index` (chance infosets only).
  Number GetActionProb(int p_index) const;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLEDITMOVE_H
