//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dleditstrategies.h
// Dialog for viewing and editing the strategies of a player
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

#ifndef GAMBIT_GUI_DLEDITSTRATEGIES_H
#define GAMBIT_GUI_DLEDITSTRATEGIES_H

#include "editlabel.h"

namespace Gambit::GUI {
class StrategyPanel;

class EditStrategiesDialog final : public wxDialog {
  GamePlayer m_player;
  StrategyPanel *m_strategyPanel;
  wxStaticText *m_errorText;

  void UpdateValidation();

public:
  // Lifecycle
  EditStrategiesDialog(wxWindow *p_parent, const GamePlayer &p_player);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  // The number of strategy rows currently in the panel, after any adds/deletes/reorders.
  // Includes rows marked deleted; check IsDeleted() before committing one.
  int NumStrategies() const;
  // True if the row at position `p_index` is marked for deletion (kept in the panel, shown
  // disabled, but excluded from the operation this dialog commits).
  bool IsDeleted(int p_index) const;
  // The label the strategy at position `p_index` had (or the placeholder it was assigned,
  // if newly added) before any edits made in this dialog.  Identifies the strategy for the
  // structural part of the operation, independent of anything typed into its label field.
  std::string GetStableLabel(int p_index) const;
  // The label as currently typed into the row at position `p_index`.
  wxString GetStrategyLabel(int p_index) const;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLEDITSTRATEGIES_H
