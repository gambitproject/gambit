//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/labelcell.cc
// Implementation of wxGrid editor for Gambit labels
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

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "labelcell.h"

namespace Gambit::GUI {

//!
//! GetCellSize() only reports what's directly set on a cell's own attr; it
//! does not detect "am I covered by an earlier cell's forward-reaching
//! span" (confirmed empirically -- it returns CellSpan_None, not
//! CellSpan_Inside, for a covered cell in this codebase's GetAttr()-computed
//! span design, where only the anchor cell's attr ever gets an explicit
//! SetSize()). So the anchor has to be found by scanning backward and
//! checking whether an earlier Main-kind cell's span reaches forward far
//! enough to cover (row, col).
//!
static void ResolveSpanAnchor(wxGrid *p_grid, int &p_row, int &p_col)
{
  for (int row = p_row; row >= 0; --row) {
    int numRows, numCols;
    if (p_grid->GetCellSize(row, p_col, &numRows, &numCols) == wxGrid::CellSpan_Main) {
      if (row + numRows > p_row) {
        p_row = row;
      }
      break;
    }
  }

  for (int col = p_col; col >= 0; --col) {
    int numRows, numCols;
    if (p_grid->GetCellSize(p_row, col, &numRows, &numCols) == wxGrid::CellSpan_Main) {
      if (col + numCols > p_col) {
        p_col = col;
      }
      break;
    }
  }
}

void LabelCellEditor::Create(wxWindow *parent, wxWindowID id, wxEvtHandler *evtHandler)
{
  auto *textCtrl = new LabelTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                     wxTE_PROCESS_TAB | wxTE_CENTER | wxBORDER_NONE);
  SetWindow(textCtrl);
  if (evtHandler) {
    textCtrl->PushEventHandler(evtHandler);
  }
}

void LabelCellEditor::BeginEdit(int row, int col, wxGrid *grid)
{
  m_grid = grid;
  m_row = row;
  m_col = col;
  ResolveSpanAnchor(grid, m_row, m_col);

  m_startValue = grid->GetTable()->GetValue(m_row, m_col);
  Text()->SetValue(m_startValue);
  // In case SetSize() already ran (before m_grid was set above) with a rect
  // that isn't span-aware, apply the correct one now too.
  Text()->SetSize(grid->CellToRect(m_row, m_col));
  Text()->SelectAll();
  Text()->SetFocus();
}

bool LabelCellEditor::EndEdit(int, int, const wxGrid *, const wxString &oldval, wxString *newval)
{
  const wxString value = Text()->GetNormalizedValue();

  if (value.empty()) {
    wxBell();
    Text()->SetFocus();
    return false;
  }

  if (value == oldval) {
    return false;
  }

  m_newValue = value;
  *newval = value;
  return true;
}

void LabelCellEditor::ApplyEdit(int row, int col, wxGrid *grid)
{
  grid->GetTable()->SetValue(row, col, m_newValue);
}

void LabelCellEditor::Reset() { Text()->SetValue(m_startValue); }

void LabelCellEditor::SetSize(const wxRect &)
{
  if (m_grid) {
    Text()->SetSize(m_grid->CellToRect(m_row, m_col));
  }
}

void LabelCellEditor::StartingKey(wxKeyEvent &event)
{
  const wxChar ch = event.GetUnicodeKey();
  if (ch != WXK_NONE) {
    Text()->SetValue(wxString(ch));
    Text()->SetInsertionPointEnd();
  }
  else {
    event.Skip();
  }
}

} // namespace Gambit::GUI
