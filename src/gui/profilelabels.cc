//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/profilelabels.cc
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "profilelabels.h"

namespace Gambit::GUI {

namespace {

//!
//! A header label renderer which fills the label with the grid's selection
//! colour before drawing it as usual.  wxGrid calls DrawBorder() for an
//! ordinary label and DrawHighlighted() for the label of the row or column
//! the grid cursor is on; as the profile lists suppress the latter, the
//! highlighting is done here.
//!
template <class Base> class HighlightedLabel final : public Base {
public:
  void DrawBorder(const wxGrid &p_grid, wxDC &p_dc, wxRect &p_rect) const override
  {
    const wxColour background = p_grid.GetSelectionBackground();
    p_dc.SetPen(*wxTRANSPARENT_PEN);
    p_dc.SetBrush(wxBrush(background.ChangeLightness(130)));
    p_dc.DrawRectangle(p_rect);
    Base::DrawBorder(p_grid, p_dc, p_rect);
  }
};

// These are stateless, so one instance of each serves every profile list.
const HighlightedLabel<wxGridColumnHeaderRendererDefault> s_highlightedColumn;
const HighlightedLabel<wxGridRowHeaderRendererDefault> s_highlightedRow;

} // end anonymous namespace

const wxGridColumnHeaderRenderer &ProfileLabelProvider::GetColumnHeaderRenderer(int p_col)
{
  if (p_col == m_col) {
    return s_highlightedColumn;
  }
  return wxGridCellAttrProvider::GetColumnHeaderRenderer(p_col);
}

const wxGridRowHeaderRenderer &ProfileLabelProvider::GetRowHeaderRenderer(int p_row)
{
  if (p_row == m_row) {
    return s_highlightedRow;
  }
  return wxGridCellAttrProvider::GetRowHeaderRenderer(p_row);
}

} // namespace Gambit::GUI
