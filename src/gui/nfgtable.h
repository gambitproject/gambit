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

class StrategicTableLayout {
  GameDocument *m_doc;
  std::vector<int> m_rowPlayers;
  std::vector<int> m_colPlayers;

  static int NumStrategies(const StrategySupportProfile &profile, int player)
  {
    return profile.GetStrategies(profile.GetGame()->GetPlayer(player)).size();
  }

  static GameStrategy LookupStrategy(const StrategySupportProfile &profile, int player,
                                     int strategy)
  {
    auto strategies = profile.GetStrategies(profile.GetGame()->GetPlayer(player));
    return *std::next(strategies.begin(), strategy - 1);
  }

public:
  explicit StrategicTableLayout(GameDocument *doc) : m_doc(doc)
  {
    if (m_doc->NumPlayers() >= 1) {
      m_rowPlayers.push_back(1);
    }
    if (m_doc->NumPlayers() >= 2) {
      m_colPlayers.push_back(2);
    }
    for (int pl = 3; pl <= m_doc->NumPlayers(); ++pl) {
      m_rowPlayers.push_back(pl);
    }
  }

  GameDocument *GetDocument() const { return m_doc; }

  /// Returns the number of players assigned to the rows
  int NumRowPlayers() const { return m_rowPlayers.size(); }

  /// Returns the index'th player assigned to the rows (1=slowest incrementing)
  int GetRowPlayer(int index) const { return m_rowPlayers[index - 1]; }

  /// Returns the number of players assigned to the columns
  int NumColPlayers() const { return m_colPlayers.size(); }

  /// Returns the index'th player assigned to the columns (1=slowest)
  int GetColPlayer(int index) const { return m_colPlayers[index - 1]; }

  void ReconcilePlayers()
  {
    const int maxPlayer = static_cast<int>(m_doc->NumPlayers());

    m_rowPlayers.erase(std::remove_if(m_rowPlayers.begin(), m_rowPlayers.end(),
                                      [maxPlayer](int pl) { return pl > maxPlayer; }),
                       m_rowPlayers.end());

    m_colPlayers.erase(std::remove_if(m_colPlayers.begin(), m_colPlayers.end(),
                                      [maxPlayer](int pl) { return pl > maxPlayer; }),
                       m_colPlayers.end());

    for (int pl = 1; pl <= maxPlayer; ++pl) {
      if (!contains(m_rowPlayers, pl) && !contains(m_colPlayers, pl)) {
        m_rowPlayers.push_back(pl);
      }
    }
  }

  void SetRowPlayer(int index, int pl)
  {
    if (contains(m_colPlayers, pl)) {
      m_colPlayers.erase(std::find(m_colPlayers.begin(), m_colPlayers.end(), pl));
    }

    if (contains(m_rowPlayers, pl)) {
      m_rowPlayers.erase(std::find(m_rowPlayers.begin(), m_rowPlayers.end(), pl));
    }

    index = std::max(1, index);
    index = std::min(index, static_cast<int>(m_rowPlayers.size()) + 1);

    auto it = m_rowPlayers.begin();
    std::advance(it, index - 1);
    m_rowPlayers.insert(it, pl);
  }

  void SetColPlayer(int index, int pl)
  {
    if (contains(m_rowPlayers, pl)) {
      m_rowPlayers.erase(std::find(m_rowPlayers.begin(), m_rowPlayers.end(), pl));
    }

    if (contains(m_colPlayers, pl)) {
      m_colPlayers.erase(std::find(m_colPlayers.begin(), m_colPlayers.end(), pl));
    }

    index = std::max(1, index);
    index = std::min(index, static_cast<int>(m_colPlayers.size()) + 1);

    auto it = m_colPlayers.begin();
    std::advance(it, index - 1);
    m_colPlayers.insert(it, pl);
  }

  int NumRowContingencies() const
  {
    int ncont = 1;
    const StrategySupportProfile &support = m_doc->GetNfgSupport();
    for (int i = 1; i <= NumRowPlayers(); ++i) {
      ncont *= NumStrategies(support, GetRowPlayer(i));
    }
    return ncont;
  }

  int NumColContingencies() const
  {
    int ncont = 1;
    const StrategySupportProfile &support = m_doc->GetNfgSupport();
    for (int i = 1; i <= NumColPlayers(); ++i) {
      ncont *= NumStrategies(support, GetColPlayer(i));
    }
    return ncont;
  }

  int NumRowsSpanned(int index) const
  {
    int ncont = 1;
    const StrategySupportProfile &support = m_doc->GetNfgSupport();
    for (int i = index + 1; i <= NumRowPlayers(); ++i) {
      ncont *= NumStrategies(support, GetRowPlayer(i));
    }
    return ncont;
  }

  int NumColsSpanned(int index) const
  {
    int ncont = 1;
    const StrategySupportProfile &support = m_doc->GetNfgSupport();
    for (int i = index + 1; i <= NumColPlayers(); ++i) {
      ncont *= NumStrategies(support, GetColPlayer(i));
    }
    return ncont;
  }

  int RowToStrategy(int player, int row) const
  {
    const int strat = row / NumRowsSpanned(player);
    return (strat % NumStrategies(m_doc->GetNfgSupport(), GetRowPlayer(player)) + 1);
  }

  int ColToStrategy(int player, int col) const
  {
    const int strat = col / m_doc->NumPlayers() / NumColsSpanned(player);
    return (strat % NumStrategies(m_doc->GetNfgSupport(), GetColPlayer(player)) + 1);
  }

  int GetRowHeaderRowCount() const { return NumRowContingencies(); }
  int GetRowHeaderColCount() const { return NumRowPlayers(); }

  int GetColHeaderRowCount() const { return NumColPlayers(); }
  int GetColHeaderColCount() const { return NumColContingencies() * m_doc->NumPlayers(); }

  int GetPayoffRowCount() const { return NumRowContingencies(); }
  int GetPayoffColCount() const { return NumColContingencies() * m_doc->NumPlayers(); }

  int GetRowHeaderPlayer(int headerCol) const { return GetRowPlayer(headerCol + 1); }

  int GetColHeaderPlayer(int headerRow) const { return GetColPlayer(headerRow + 1); }

  int GetRowHeaderStrategy(int headerCol, int headerRow) const
  {
    return RowToStrategy(headerCol + 1, headerRow);
  }

  int GetColHeaderStrategy(int headerRow, int headerCol) const
  {
    return ColToStrategy(headerRow + 1, headerCol);
  }

  int GetRowHeaderRowSpan(int headerCol) const { return NumRowsSpanned(headerCol + 1); }

  int GetColHeaderColSpan(int headerRow) const
  {
    return NumColsSpanned(headerRow + 1) * m_doc->NumPlayers();
  }

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

  PureStrategyProfile CellToProfile(const wxSheetCoords &coords) const
  {
    const StrategySupportProfile &support = m_doc->GetNfgSupport();

    const PureStrategyProfile profile = m_doc->GetGame()->NewPureStrategyProfile();
    for (int i = 1; i <= NumRowPlayers(); ++i) {
      const int player = GetRowPlayer(i);
      profile->SetStrategy(LookupStrategy(support, player, RowToStrategy(i, coords.GetRow())));
    }

    for (int i = 1; i <= NumColPlayers(); ++i) {
      const int player = GetColPlayer(i);
      profile->SetStrategy(LookupStrategy(support, player, ColToStrategy(i, coords.GetCol())));
    }

    return profile;
  }

  PureStrategyProfile GetPayoffProfile(const wxSheetCoords &coords) const
  {
    return CellToProfile(coords);
  }
};

//!
//! This is a panel which manages three wxSheet instances: one which
//! contains the payoffs of the strategic form, and two which handle
//! the display of row and column labels
//!
class TableWidget final : public wxPanel {
  GameDocument *m_doc;
  NfgPanel *m_nfgPanel;
  wxSheet *m_payoffSheet, *m_rowSheet, *m_colSheet;

  std::shared_ptr<StrategicTableLayout> m_layout;

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
  int NumRowPlayers() const { return m_layout->NumRowPlayers(); }

  /// Returns the index'th player assigned to the rows (1=slowest incrementing)
  int GetRowPlayer(int index) const { return m_layout->GetRowPlayer(index); }

  /// Sets the index'th row player (1=slowest, n+1=fastest)
  void SetRowPlayer(int index, int pl);

  /// Returns the number of row contingencies (i.e., rows in the table)
  int NumRowContingencies() const { return m_layout->NumRowContingencies(); }

  /// Returns the number of rows spanned by strategy of row player index
  int NumRowsSpanned(int index) const { return m_layout->NumRowsSpanned(index); }

  /// Returns the strategy index for row player 'player' corresponding to 'row'
  int RowToStrategy(int player, int row) const { return m_layout->RowToStrategy(player, row); }

  /// Returns the number of players assigned to the columns
  int NumColPlayers() const { return m_layout->NumColPlayers(); }

  /// Returns the index'th player assigned to the columns (1=slowest)
  int GetColPlayer(int index) const { return m_layout->GetColPlayer(index); }

  /// Sets the index'th column player (1=slowest, n+1=fastest)
  void SetColPlayer(int index, int pl);

  /// Returns the number of column contingencies
  /// (Note that each column contingency corresponds to #players cols!)
  int NumColContingencies() const { return m_layout->NumColContingencies(); }

  /// Returns the number of columns spanned by strategy of column player index
  int NumColsSpanned(int index) const { return m_layout->NumColsSpanned(index); }

  /// Returns the strategy index for row player 'player' corresponding to 'row'
  int ColToStrategy(int player, int row) const { return m_layout->ColToStrategy(player, row); }

  /// Returns the strategy profile corresponding to a cell
  PureStrategyProfile CellToProfile(const wxSheetCoords &p_coords) const
  {
    return m_layout->CellToProfile(p_coords);
  }
  //@}

  //@{
  // Header-pane dimensions
  int GetRowHeaderRowCount() const { return m_layout->GetRowHeaderRowCount(); }
  int GetRowHeaderColCount() const { return m_layout->GetRowHeaderColCount(); }

  int GetColHeaderRowCount() const { return m_layout->GetColHeaderRowCount(); }
  int GetColHeaderColCount() const { return m_layout->GetColHeaderColCount(); }

  // Payoff-pane dimensions
  int GetPayoffRowCount() const { return m_layout->GetPayoffRowCount(); }
  int GetPayoffColCount() const { return m_layout->GetPayoffColCount(); }

  // Header-pane player mapping
  int GetRowHeaderPlayer(int headerCol) const { return m_layout->GetRowHeaderPlayer(headerCol); }
  int GetColHeaderPlayer(int headerRow) const { return m_layout->GetColHeaderPlayer(headerRow); }

  // Header-pane strategy mapping
  int GetRowHeaderStrategy(int headerCol, int headerRow) const
  {
    return m_layout->GetRowHeaderStrategy(headerCol, headerRow);
  }

  int GetColHeaderStrategy(int headerRow, int headerCol) const
  {
    return m_layout->GetColHeaderStrategy(headerRow, headerCol);
  }

  // Header-pane span mapping
  int GetRowHeaderRowSpan(int headerCol) const { return m_layout->GetRowHeaderRowSpan(headerCol); }

  int GetColHeaderColSpan(int headerRow) const { return m_layout->GetColHeaderColSpan(headerRow); }

  // Payoff-pane mapping helpers
  int GetPayoffPlayerForColumn(int payoffCol) const
  {
    return m_layout->GetPayoffPlayerForColumn(payoffCol);
  }

  GameStrategy GetStrategyByPlayerAndIndex(int player, int strategy) const;

  PureStrategyProfile GetPayoffProfile(const wxSheetCoords &coords) const
  {
    return CellToProfile(coords);
  }

  GamePlayer GetPayoffPlayer(int payoffCol) const;
  int GetPayoffColumnsPerContingency() const;
  bool IsPayoffStrategyDominated(const wxSheetCoords &coords, bool strict) const;

  //@}

  bool IsReadOnly() const;
  wxColour GetPlayerColor(int player) const;
  const StrategySupportProfile &GetSupport() const { return m_doc->GetNfgSupport(); }

  bool IsRowHeaderStrategyDominated(int headerCol, int headerRow, bool strict) const;
  bool IsColHeaderStrategyDominated(int headerRow, int headerCol, bool strict) const;

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
  void RenameRowHeaderStrategy(int headerCol, int headerRow, const wxString &value);
  void RenameColHeaderStrategy(int headerRow, int headerCol, const wxString &value);
  void DeleteRowHeaderStrategy(int headerCol, int headerRow);
  void DeleteColHeaderStrategy(int headerRow, int headerCol);
  void SetPayoffCellValue(const wxSheetCoords &coords, const wxString &value);
  //@}
};
} // namespace Gambit::GUI
#endif // GAMBIT_GUI_NFGTABLE_H
