//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgprofile.h
// Extensive form behavior profile window
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

#ifndef GAMBIT_GUI_EFGPROFILE_H
#define GAMBIT_GUI_EFGPROFILE_H

#include <wx/grid.h>

#include "gamedoc.h"

namespace Gambit::GUI {
class BehaviorProfileList final : public wxGrid, public GameView {
  // Event handlers
  void OnLabelClick(wxGridEvent &);
  void OnCellClick(wxGridEvent &);
  void OnSelectCell(wxGridEvent &);

  void ResizeGrid(int p_rows, int p_cols);
  void UpdateLabels();
  void UpdateCells();

  // Overriding GameView members
  void OnUpdate() override;

public:
  BehaviorProfileList(wxWindow *p_parent, GameDocument *p_doc);
  ~BehaviorProfileList() override;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_EFGPROFILE_H
