//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/renratio.h
// Declaration of wxGrid renderer/editor for rational numbers
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

#ifndef GAMBIT_GUI_RENRATIO_H
#define GAMBIT_GUI_RENRATIO_H

#include <wx/grid.h>

namespace Gambit::GUI {

//!
//! Renderer for payoff cells: draws values containing '/' as a stacked
//! numerator/denominator fraction, everything else as plain text.
//!
class RationalCellRenderer : public wxGridCellStringRenderer {
public:
  RationalCellRenderer() = default;

  void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col,
            bool isSelected) override;
  wxSize GetBestSize(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, int row, int col) override;

  wxGridCellRenderer *Clone() const override { return new RationalCellRenderer(); }

private:
  static void SetTextColoursAndFont(wxGrid &grid, const wxGridCellAttr &attr, wxDC &dc,
                                    bool isSelected);
  static wxSize DoGetBestSize(wxGrid &grid, const wxGridCellAttr &attr, wxDC &dc,
                              const wxString &text);
};

//!
//! Editor for payoff cells: restricts input to digits, a single decimal
//! point, and a leading minus sign.
//!
class RationalCellEditor final : public wxGridCellTextEditor {
public:
  RationalCellEditor() = default;

  void Create(wxWindow *parent, wxWindowID id, wxEvtHandler *evtHandler) override;
  bool IsAcceptedKey(wxKeyEvent &event) override;
  void StartingKey(wxKeyEvent &event) override;

  wxGridCellEditor *Clone() const override { return new RationalCellEditor(); }
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_RENRATIO_H
