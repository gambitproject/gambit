//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/nfgtable.h
// Declaration of control to display/edit strategic game tables
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

#ifndef GAMBIT_GUI_NFGTABLE_H
#define GAMBIT_GUI_NFGTABLE_H

namespace Gambit::GUI {
class GameDocument;
class NfgPanel;

//!
//! This is a panel which manages three wxSheet instances: one which
//! contains the payoffs of the strategic form, and two which handle
//! the display of row and column labels
//!
class TableWidget final : public wxPanel {
  GameDocument *m_doc;
  NfgPanel *m_nfgPanel;
  wxSheet *m_payoffSheet, *m_rowSheet, *m_colSheet;

  Array<int> m_rowPlayers, m_colPlayers;

  /// @name Event handlers
  //@{
  /// Called when row label sheet is scrolled
  void OnRowSheetScroll(wxSheetEvent &);
  /// Called when column label sheet is scrolled
  void OnColSheetScroll(wxSheetEvent &);
  /// Called when payoff sheet is scrolled
  void OnPayoffScroll(wxSheetEvent &);

  /// Called when row label sheet row is resized
  void OnRowSheetRow(wxSheetEvent &);
  /// Called when payoff sheet row is resized
  void OnPayoffRow(wxSheetEvent &);

  /// Called when col label sheet column is resized
  void OnColSheetColumn(wxSheetEvent &);
  /// Called when payoff sheet column is resized
  void OnPayoffColumn(wxSheetEvent &);

  /// Called when row label sheet column is resized
  void OnRowSheetColumn(wxSheetEvent &);
  /// Called when column label sheet row is resized
  void OnColSheetRow(wxSheetEvent &);

  /// Called when editing begins in any cell
  void OnBeginEdit(wxSheetEvent &);
  void ReconcilePlayers();
  void UpdatePayoffPanel();
  void UpdateLabelPanelMargins();
  void UpdateLabelPanels();
  //@}

  int GetRowPaneWidth() const;
  int GetColPaneHeight() const;
  void UpdateLabelPanelSizes();

public:
  TableWidget(NfgPanel *p_parent, wxWindowID p_id, GameDocument *p_doc);

  /// @name Coordination of sheets
  //@{
  /// Synchronize with document state
  void OnUpdate();

  /// Post any pending edits
  void PostPendingChanges();

  /// Are we showing dominance indicators or not?
  bool ShowDominance() const;
  //@}

  /// @name View state
  //@{
  /// Returns the number of players assigned to the rows
  int NumRowPlayers() const { return m_rowPlayers.size(); }

  /// Returns the index'th player assigned to the rows (1=slowest incrementing)
  int GetRowPlayer(int index) const { return m_rowPlayers[index]; }

  /// Sets the index'th row player (1=slowest, n+1=fastest)
  void SetRowPlayer(int index, int pl);

  /// Returns the number of row contingencies (i.e., rows in the table)
  int NumRowContingencies() const;

  /// Returns the number of rows spanned by strategy of row player index
  int NumRowsSpanned(int index) const;

  /// Returns the strategy index for row player 'player' corresponding to 'row'
  int RowToStrategy(int player, int row) const;

  /// Returns the number of players assigned to the columns
  int NumColPlayers() const { return m_colPlayers.size(); }

  /// Returns the index'th player assigned to the columns (1=slowest)
  int GetColPlayer(int index) const { return m_colPlayers[index]; }

  /// Sets the index'th column player (1=slowest, n+1=fastest)
  void SetColPlayer(int index, int pl);

  /// Returns the number of column contingencies
  /// (Note that each column contingency corresponds to #players cols!)
  int NumColContingencies() const;

  /// Returns the number of columns spanned by strategy of column player index
  int NumColsSpanned(int index) const;

  /// Returns the strategy index for row player 'player' corresponding to 'row'
  int ColToStrategy(int player, int row) const;

  /// Returns the strategy profile corresponding to a cell
  PureStrategyProfile CellToProfile(const wxSheetCoords &) const;
  //@}

  //@{
  // Header-pane dimensions
  int GetRowHeaderRowCount() const { return NumRowContingencies(); }
  int GetRowHeaderColCount() const { return NumRowPlayers(); }

  int GetColHeaderRowCount() const { return NumColPlayers(); }
  int GetColHeaderColCount() const { return NumColContingencies() * m_doc->NumPlayers(); }

  // Payoff-pane dimensions
  int GetPayoffRowCount() const { return NumRowContingencies(); }
  int GetPayoffColCount() const { return NumColContingencies() * m_doc->NumPlayers(); }

  // Header-pane player mapping
  int GetRowHeaderPlayer(int headerCol) const { return GetRowPlayer(headerCol + 1); }
  int GetColHeaderPlayer(int headerRow) const { return GetColPlayer(headerRow + 1); }

  // Header-pane strategy mapping
  int GetRowHeaderStrategy(int headerCol, int headerRow) const
  {
    return RowToStrategy(headerCol + 1, headerRow);
  }

  int GetColHeaderStrategy(int headerRow, int headerCol) const
  {
    return ColToStrategy(headerRow + 1, headerCol);
  }

  // Header-pane span mapping
  int GetRowHeaderRowSpan(int headerCol) const { return NumRowsSpanned(headerCol + 1); }

  int GetColHeaderColSpan(int headerRow) const
  {
    return NumColsSpanned(headerRow + 1) * m_doc->NumPlayers();
  }

  // Payoff-pane mapping helpers
  int GetPayoffPlayerForColumn(int payoffCol) const
  {
    const int index = payoffCol % m_doc->NumPlayers() + 1;
    if (index <= NumRowPlayers()) {
      return GetRowPlayer(index);
    }
    else {
      return GetColPlayer(index - NumRowPlayers());
    }
  }

  PureStrategyProfile GetPayoffProfile(const wxSheetCoords &coords) const
  {
    return CellToProfile(coords);
  }
  //@}

  /// @name Exporting/printing graphics
  //@{
  /// Creates a printout object of the game as currently displayed
  wxPrintout *GetPrintout();
  /// Creates a bitmap of the game as currently displayed
  bool GetBitmap(wxBitmap &, int marginX, int marginY);
  /// Outputs the game as currently displayed to a SVG file
  void GetSVG(const wxString &p_filename, int marginX, int marginY);
  /// Prints the game as currently displayed, centered on the DC
  void RenderGame(wxDC &p_dc, int marginX, int marginY);
  //@}
};
} // namespace Gambit::GUI
#endif // GAMBIT_GUI_NFGTABLE_H
