//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/profilelabels.h
// Highlighting of the row and column labels in the profile list windows
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

#ifndef GAMBIT_GUI_PROFILELABELS_H
#define GAMBIT_GUI_PROFILELABELS_H

#include <wx/grid.h>

namespace Gambit::GUI {

//!
//! Draws the labels of a profile list, highlighting the column the list is
//! sorted on and the row showing the currently selected profile.
//!
//! Left to itself, wxGrid highlights the labels of the row and column the
//! grid cursor happens to be on.  The cursor carries no meaning in a profile
//! list, and cannot be moved out of the way, as wxGrid places it back on the
//! first cell whenever the grid is repainted.  The profile lists therefore
//! turn that highlighting off with DisableOverlaySelection(), and use this
//! provider to highlight the labels which do carry meaning instead.
//!
//! Rows and columns are numbered from zero, following the convention of
//! wxGrid.  An instance is owned by the grid's table, which deletes it.
//!
class ProfileLabelProvider final : public wxGridCellAttrProvider {
public:
  /// Highlight the label of column p_col, or of no column if -1
  void SetSortedColumn(int p_col) { m_col = p_col; }

  /// Highlight the label of row p_row, or of no row if -1
  void SetSelectedRow(int p_row) { m_row = p_row; }

  const wxGridColumnHeaderRenderer &GetColumnHeaderRenderer(int p_col) override;
  const wxGridRowHeaderRenderer &GetRowHeaderRenderer(int p_row) override;

private:
  int m_col{-1};
  int m_row{-1};
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_PROFILELABELS_H
