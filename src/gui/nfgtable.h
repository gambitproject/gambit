//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

#ifndef NFGTABLE_H
#define NFGTABLE_H

class gbtGameDocument;
class gbtNfgPanel;

//!
//! This is a panel which manages three wxSheet instances: one which
//! contains the payoffs of the strategic form, and two which handle
//! the display of row and column labels
//!
class gbtTableWidget : public wxPanel {
private:
  gbtGameDocument *m_doc;
  gbtNfgPanel *m_nfgPanel;
  wxSheet *m_payoffSheet, *m_rowSheet, *m_colSheet;

  Gambit::Array<int> m_rowPlayers, m_colPlayers;

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
  //@}

public:
  gbtTableWidget(gbtNfgPanel *p_parent, wxWindowID p_id,
		 gbtGameDocument *p_doc);

  /// @name Coordination of sheets
  //@{
  /// Scroll row and column label sheets
  void SetGridOrigin(int x, int y);

  /// Synchronize with document state
  void OnUpdate(void);

  /// Post any pending edits
  void PostPendingChanges(void);

  /// Are we showing dominance indicators or not?
  bool ShowDominance(void) const;
  //@}

  /// @name View state
  //@{
  /// Returns the number of players assigned to the rows
  int NumRowPlayers(void) const { return m_rowPlayers.Length(); }

  /// Returns the index'th player assigned to the rows (1=slowest incrementing)
  int GetRowPlayer(int index) const { return m_rowPlayers[index]; }

  /// Sets the index'th row player (1=slowest, n+1=fastest)
  void SetRowPlayer(int index, int pl);

  /// Returns the number of row contingencies (i.e., rows in the table)
  int NumRowContingencies(void) const;

  /// Returns the number of rows spanned by strategy of row player index
  int NumRowsSpanned(int index) const;

  /// Returns the strategy index for row player 'player' corresponding to 'row'
  int RowToStrategy(int player, int row) const;

  /// Returns the number of players assigned to the columns
  int NumColPlayers(void) const { return m_colPlayers.Length(); }

  /// Returns the index'th player assigned to the columns (1=slowest)
  int GetColPlayer(int index) const { return m_colPlayers[index]; }

  /// Sets the index'th column player (1=slowest, n+1=fastest)
  void SetColPlayer(int index, int pl);

  /// Returns the number of column contingencies 
  /// (Note that each column contingency corresponds to #players cols!)
  int NumColContingencies(void) const;

  /// Returns the number of columns spanned by strategy of column player index
  int NumColsSpanned(int index) const;

  /// Returns the strategy index for row player 'player' corresponding to 'row'
  int ColToStrategy(int player, int row) const;

  /// Returns the strategy profile corresponding to a cell
  Gambit::PureStrategyProfile CellToProfile(const wxSheetCoords &) const;
  //@}

  /// @name Exporting/printing graphics
  //@{
  /// Creates a printout object of the game as currently displayed
  wxPrintout *GetPrintout(void);
  /// Creates a bitmap of the game as currently displayed
  bool GetBitmap(wxBitmap &, int marginX, int marginY);
  /// Outputs the game as currently displayed to a SVG file
  void GetSVG(const wxString &p_filename, int marginX, int marginY);
  /// Prints the game as currently displayed, centered on the DC
  void RenderGame(wxDC &p_dc, int marginX, int marginY);
  //@}
};

#endif  // NFGTABLE_H
