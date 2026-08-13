//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/labelcell.h
// Declaration of wxGrid editor for Gambit labels
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

#ifndef GAMBIT_GUI_LABELCELLEDITOR_H
#define GAMBIT_GUI_LABELCELLEDITOR_H

#include <wx/grid.h>

#include "editlabel.h"

namespace Gambit::GUI {

//!
//! Cell editor for strategy/action label renaming. Wraps LabelTextCtrl,
//! which does its own live whitespace normalization; this class only
//! handles the wxGridCellEditor lifecycle (open/commit/cancel) around it.
//!
class LabelCellEditor final : public wxGridCellEditor {
  wxString m_startValue;
  wxString m_newValue;

  /// Cell this editor is currently attached to, kept so SetSize() can
  /// recompute the correct on-screen rect itself -- see the comment there.
  wxGrid *m_grid{nullptr};
  int m_row{0};
  int m_col{0};

  LabelTextCtrl *Text() const { return static_cast<LabelTextCtrl *>(GetWindow()); }

  /// Positions the editor within (row, col)'s cell: full width, natural
  /// single-line height, centered vertically. See the definition for why.
  void PositionInCell(int row, int col);

public:
  LabelCellEditor() = default;

  void Create(wxWindow *parent, wxWindowID id, wxEvtHandler *evtHandler) override;
  void BeginEdit(int row, int col, wxGrid *grid) override;
  bool EndEdit(int row, int col, const wxGrid *grid, const wxString &oldval,
               wxString *newval) override;
  void ApplyEdit(int row, int col, wxGrid *grid) override;
  void Reset() override;

  /// wxGrid's own editor-positioning logic isn't aware of merged (spanned)
  /// cells the way CellToRect() is, so the correct rect for a spanned
  /// header cell is recomputed here directly rather than trusting whatever
  /// rect wxGrid passes in.
  void SetSize(const wxRect &rect) override;

  /// Injects the keystroke that triggered editing (typing directly on a
  /// selected cell, rather than clicking/F2) into the editor.
  void StartingKey(wxKeyEvent &event) override;

  wxGridCellEditor *Clone() const override { return new LabelCellEditor(); }
  wxString GetValue() const override { return Text()->GetValue(); }
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_LABELCELLEDITOR_H
