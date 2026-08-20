//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/nfgpayoffgrid.cc
// The payoff-matrix grid for the strategic game matrix display
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

#include <wx/grid.h>

#include "renratio.h" // special renderer/editor for rational numbers

#include "gamedoc.h"
#include "nfgtable.h"

namespace Gambit::GUI {

//=========================================================================
//                       class PayoffCellRenderer
//=========================================================================

//!
//! Draws payoff cells: rational-number rendering (from RationalCellRenderer),
//! regular-weight black lines marking contingency/row boundaries (with the
//! grid's own light-gray default gridlines left to separate the columns
//! within a single contingency), and the dominance-indicator overlay.
//!
class PayoffCellRenderer final : public RationalCellRenderer {
  TableWidget *m_table;

  //!
  //! Draws a 1px black line inset within this cell's own rect (not at the
  //! exact shared pixel with the next cell/row) so it survives painting
  //! regardless of draw order between adjacent cells: the neighbouring
  //! cell's own background fill covers only its own rect and never reaches
  //! back into pixels this cell already owns, whereas a line drawn exactly
  //! at the shared boundary pixel sits in the neighbour's own territory and
  //! gets erased by its fill.
  //!
  static void DrawInsetLine(wxDC &dc, int x1, int y1, int x2, int y2)
  {
    dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));
    dc.DrawLine(x1, y1, x2, y2);
  }

public:
  explicit PayoffCellRenderer(TableWidget *p_table) : m_table(p_table) {}

  void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col,
            bool isSelected) override
  {
    RationalCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

    // Vertical: black at contingency boundaries (grid's own light-gray
    // default gridlines already separate the columns within a contingency).
    const int perContingency = m_table->GetPayoffColumnsPerContingency();
    if ((col + 1) % perContingency == 0) {
      const int x = rect.x + rect.width - 1;
      DrawInsetLine(dc, x, rect.y, x, rect.y + rect.height);
    }

    // Horizontal: every row is its own contingency, so every row boundary
    // is black -- bottom of every cell, plus the top edge for row 0 (no
    // preceding row's bottom line to serve as its top).
    DrawInsetLine(dc, rect.x, rect.y + rect.height - 1, rect.x + rect.width,
                  rect.y + rect.height - 1);
    if (row == 0) {
      DrawInsetLine(dc, rect.x, rect.y, rect.x + rect.width, rect.y);
    }

    if (!m_table->ShowDominance()) {
      return;
    }

    auto player = m_table->GetPayoffPlayer(col);

    if (m_table->IsPayoffStrategyDominated(row, col, false)) {
      if (m_table->IsPayoffStrategyDominated(row, col, true)) {
        dc.SetPen(wxPen(m_table->GetPlayerColor(player->GetNumber()), 2, wxPENSTYLE_SOLID));
      }
      else {
        dc.SetPen(wxPen(m_table->GetPlayerColor(player->GetNumber()), 1, wxPENSTYLE_SHORT_DASH));
      }
      dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
      dc.DrawLine(rect.x + rect.width, rect.y, rect.x, rect.y + rect.height);
    }
  }

  wxGridCellRenderer *Clone() const override { return new PayoffCellRenderer(m_table); }
};

//=========================================================================
//                       class PayoffTable
//=========================================================================

class PayoffTable final : public wxGridTableBase {
  TableWidget *m_table;
  int m_numRows{0};
  int m_numCols{0};
  PayoffCellRenderer *m_renderer;
  RationalCellEditor *m_editor;

public:
  explicit PayoffTable(TableWidget *p_table)
    : m_table(p_table), m_renderer(new PayoffCellRenderer(p_table)),
      m_editor(new RationalCellEditor())
  {
    m_renderer->IncRef();
    m_editor->IncRef();
  }

  ~PayoffTable() override
  {
    m_renderer->DecRef();
    m_editor->DecRef();
  }

  int GetNumberRows() override { return m_numRows; }
  int GetNumberCols() override { return m_numCols; }
  bool IsEmptyCell(int, int) override { return false; }
  wxString GetValue(int row, int col) override;
  void SetValue(int row, int col, const wxString &value) override;
  wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) override;

  void OnUpdate();
};

wxString PayoffTable::GetValue(int row, int col)
{
  const PureStrategyProfile profile = m_table->GetPayoffProfile(row, col);
  const auto player = m_table->GetPayoffPlayer(col);
  try {
    if (const auto outcome = profile->GetOutcome()) {
      return wxString::FromUTF8(outcome->GetPayoff<std::string>(player));
    }
    return wxString::FromUTF8("0");
  }
  catch (const UndefinedException &) {
    return wxString::FromUTF8(lexical_cast<std::string>(profile->GetPayoff(player)));
  }
}

void PayoffTable::SetValue(int row, int col, const wxString &value)
{
  wxString trimmed = value;
  if (trimmed.EndsWith(_T("/"))) {
    trimmed = trimmed.Left(trimmed.length() - 1);
  }
  m_table->SetPayoffCellValue(row, col, trimmed);
}

wxGridCellAttr *PayoffTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind)
{
  auto *attr = new wxGridCellAttr();
  attr->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  // wxGridCellStringRenderer's default is to let text that's too wide spill
  // into a neighbouring cell if that cell is otherwise empty -- wrong here,
  // since the "neighbour" past a span boundary is a different strategy's
  // header cell, not empty space.
  attr->SetOverflow(false);
  attr->SetTextColour(m_table->GetPlayerColor(m_table->GetPayoffPlayerForColumn(col)));
  attr->SetBackgroundColour(*wxWHITE);
  m_renderer->IncRef();
  attr->SetRenderer(m_renderer);
  m_editor->IncRef();
  attr->SetEditor(m_editor);
  attr->SetReadOnly(m_table->IsReadOnly());
  return attr;
}

void PayoffTable::OnUpdate()
{
  wxGrid *view = GetView();

  const int newCols = m_table->GetPayoffColCount();
  if (newCols > m_numCols) {
    const int added = newCols - m_numCols;
    m_numCols = newCols;
    if (view) {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, added);
      view->ProcessTableMessage(msg);
    }
  }
  else if (newCols < m_numCols) {
    const int removed = m_numCols - newCols;
    const int pos = newCols;
    m_numCols = newCols;
    if (view) {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_DELETED, pos, removed);
      view->ProcessTableMessage(msg);
    }
  }

  const int newRows = m_table->GetPayoffRowCount();
  if (newRows > m_numRows) {
    const int added = newRows - m_numRows;
    m_numRows = newRows;
    if (view) {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, added);
      view->ProcessTableMessage(msg);
    }
  }
  else if (newRows < m_numRows) {
    const int removed = m_numRows - newRows;
    const int pos = newRows;
    m_numRows = newRows;
    if (view) {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, pos, removed);
      view->ProcessTableMessage(msg);
    }
  }

  if (view) {
    view->Refresh();
  }
}

//=========================================================================
//                       class PayoffGrid
//=========================================================================

class PayoffGrid final : public TableGridBase {
  TableWidget *m_table;
  PayoffTable *m_gridTable;

  /// @name Overriding wxGrid behavior for event-handling
  //@{
  /// Implement custom tab-traversal behavior
  void OnKeyDown(wxKeyEvent &);
  void OnCharHook(wxKeyEvent &);
  void HandleTabTraversal(wxKeyEvent &);
  void MoveEditorByTab(bool p_backwards);
  //@}

public:
  explicit PayoffGrid(TableWidget *p_parent);

  void OnUpdate() { m_gridTable->OnUpdate(); }
};

PayoffGrid::PayoffGrid(TableWidget *p_parent)
  : TableGridBase(p_parent, wxID_ANY), m_table(p_parent), m_gridTable(new PayoffTable(p_parent))
{
  SetTable(m_gridTable, true);

  Bind(wxEVT_KEY_DOWN, &PayoffGrid::OnKeyDown, this);
  Bind(wxEVT_CHAR_HOOK, &PayoffGrid::OnCharHook, this);
}

void PayoffGrid::MoveEditorByTab(bool p_backwards)
{
  if (!GetNumberRows() || !GetNumberCols() || !IsCellEditControlShown()) {
    return;
  }

  DisableCellEditControl();

  int newRow = GetGridCursorRow();
  int newCol = GetGridCursorCol();

  if (p_backwards) {
    --newCol;
    if (newCol < 0) {
      newCol = GetNumberCols() - 1;
      --newRow;
      if (newRow < 0) {
        newRow = GetNumberRows() - 1;
      }
    }
  }
  else {
    ++newCol;
    if (newCol >= GetNumberCols()) {
      newCol = 0;
      ++newRow;
      if (newRow >= GetNumberRows()) {
        newRow = 0;
      }
    }
  }

  SetGridCursor(newRow, newCol);
  MakeCellVisible(newRow, newCol);

  // Deferred: creating the new cell's editor in the same call stack that's
  // still tearing down the old (focused) one is unreliable.
  CallAfter([this] { EnableCellEditControl(); });
}

void PayoffGrid::HandleTabTraversal(wxKeyEvent &p_event)
{
  if (p_event.GetKeyCode() != WXK_TAB || !IsCellEditControlShown()) {
    p_event.Skip();
    return;
  }
  MoveEditorByTab(p_event.ShiftDown());
}

void PayoffGrid::OnKeyDown(wxKeyEvent &p_event) { HandleTabTraversal(p_event); }

void PayoffGrid::OnCharHook(wxKeyEvent &p_event) { HandleTabTraversal(p_event); }

//=========================================================================
//                TableWidget: payoff grid construction/update
//=========================================================================

void TableWidget::InitPayoffGrid() { m_payoffGrid = new PayoffGrid(this); }

void TableWidget::UpdatePayoffGrid() { dynamic_cast<PayoffGrid *>(m_payoffGrid)->OnUpdate(); }
} // namespace Gambit::GUI
