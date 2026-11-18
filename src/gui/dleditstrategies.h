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
#include "labellist.h"

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
  // The strategies as edited: added, deleted, reordered and relabelled
  const LabelListEdit<> &GetStrategies() const;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLEDITSTRATEGIES_H
