//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/nfgtable.cc
// Implementation of strategic game matrix display/editor
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
#endif                // WX_PRECOMP
#include <wx/dnd.h>   // for drag-and-drop support
#include <wx/print.h> // for printing support
#include <wx/dcsvg.h> // for SVG output

#include "wx/sheet/sheet.h"

#include "renratio.h" // special renderer for rational numbers

#include "gamedoc.h"
#include "nfgpanel.h"
#include "nfgtable.h"
#include "dlexcept.h"

namespace Gambit::GUI {

//=========================================================================
//                       class TableWidgetBase
//=========================================================================

//!
//! This class handles some common overriding of wxSheet behavior
//! common to the sheets used in the strategic game display.
//!
class TableWidgetBase : public wxSheet {
protected:
  //!
  //! @name Overriding wxSheet members to disable selection behavior
  //!
  //@{
  bool SelectRow(int, bool = false, bool = false) override { return false; }
  bool SelectRows(int, int, bool = false, bool = false) override { return false; }
  bool SelectCol(int, bool = false, bool = false) override { return false; }
  bool SelectCols(int, int, bool = false, bool = false) override { return false; }
  bool SelectCell(const wxSheetCoords &, bool = false, bool = false) override { return false; }
  bool SelectBlock(const wxSheetBlock &, bool = false, bool = false) override { return false; }
  bool SelectAll(bool = false) override { return false; }

  bool HasSelection(bool = true) const override { return false; }
  bool IsCellSelected(const wxSheetCoords &) const override { return false; }
  bool IsRowSelected(int) const override { return false; }
  bool IsColSelected(int) const override { return false; }
  bool DeselectBlock(const wxSheetBlock &, bool = false) override { return false; }
  bool ClearSelection(bool = false) override { return false; }
  //@}

  /// Overriding wxSheet member to suppress drawing of cursor
  void DrawCursorCellHighlight(wxDC &, const wxSheetCellAttr &) override {}

  /// Overriding wxSheet member to show editor on one click
  void OnCellLeftClick(wxSheetEvent &);

public:
  /// @name Lifecycle
  //@{
  /// Constructor
  TableWidgetBase(wxWindow *p_parent, wxWindowID p_id, const wxPoint &p_pos = wxDefaultPosition,
                  const wxSize &p_size = wxDefaultSize, long p_style = wxWANTS_CHARS,
                  const wxString &p_name = wxT("wxSheet"))
    : wxSheet(p_parent, p_id, p_pos, p_size, p_style, p_name)
  {
    Connect(GetId(), wxEVT_SHEET_CELL_LEFT_DOWN,
            (wxObjectEventFunction) reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
                wxSheetEventFunction,
                static_cast<wxSheetEventFunction>(&TableWidgetBase::OnCellLeftClick))));
  }
  //@}

  /// @name Drop target interaction
  //@{
  /// Called when the drop target receives text.
  virtual bool DropText(int p_x, int p_y, const wxString &p_text) { return false; }
  //@}

  /// @name Access to scrollbars from underlying wxSheet
  //@{
  /// Get the vertical scrollbar
  wxScrollBar *GetVerticalScrollBar() const { return m_vertScrollBar; }
  /// Get the horizontal scrollbar
  wxScrollBar *GetHorizontalScrollBar() const { return m_horizScrollBar; }
  //@}
};

void TableWidgetBase::OnCellLeftClick(wxSheetEvent &p_event)
{
  SetGridCursorCell(p_event.GetCoords());
  EnableCellEditControl(p_event.GetCoords());
}

//=========================================================================
//                class gbtTableWidgetDropTarget
//=========================================================================

//!
//! This simple class serves as a drop target for players; it simply
//! communicates the location and text of the drop to its owner for
//! further processing
//!
class gbtTableWidgetDropTarget : public wxTextDropTarget {
private:
  TableWidgetBase *m_owner;

public:
  explicit gbtTableWidgetDropTarget(TableWidgetBase *p_owner) : m_owner(p_owner) {}

  bool OnDropText(wxCoord x, wxCoord y, const wxString &p_text) override
  {
    return m_owner->DropText(x, y, p_text);
  }
};

//=========================================================================
//                       class RowPlayerWidget
//=========================================================================

class RowPlayerWidget final : public TableWidgetBase {
  TableWidget *m_table;

  /// @name Overriding wxSheet members for data access
  //@{
  /// Returns the value in the cell
  wxString GetCellValue(const wxSheetCoords &) override;
  /// Sets the value in the cell, by relabeling the strategy
  void SetCellValue(const wxSheetCoords &, const wxString &) override;
  /// Returns the attributes of the cell
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const override;
  //@}

  /// @name Overriding wxSheet members to customize drawing
  //@{
  /// Overrides to draw dominance indicators
  void DrawCell(wxDC &p_dc, const wxSheetCoords &p_coords) override;
  //@}

  void OnCellRightClick(wxSheetEvent &);

public:
  /// @name Lifecycle
  //@{
  /// Constructorw
  RowPlayerWidget(TableWidget *p_parent);
  //@}

  /// @name Synchronizing with document state
  //@{
  void OnUpdate();
  //@}

  /// @name Drop target interaction
  //@{
  /// Called when the drop target receives text.
  bool DropText(int p_x, int p_y, const wxString &p_text) override;
  //@}
};

RowPlayerWidget::RowPlayerWidget(TableWidget *p_parent)
  : TableWidgetBase(p_parent, wxID_ANY), m_table(p_parent)
{
  CreateGrid(m_table->NumRowContingencies(), m_table->NumRowPlayers());
  SetRowLabelWidth(1);
  SetColLabelHeight(1);
  SetScrollBarMode(SB_NEVER);
  SetGridLineColour(*wxBLACK);

  wxWindow::SetDropTarget(new gbtTableWidgetDropTarget(this));

  Connect(GetId(), wxEVT_SHEET_CELL_RIGHT_DOWN,
          reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction,
              static_cast<wxSheetEventFunction>(&RowPlayerWidget::OnCellRightClick))));
}

void RowPlayerWidget::OnCellRightClick(wxSheetEvent &p_event)
{
  if (m_table->GetRowHeaderColCount() == 0 || m_table->IsReadOnly()) {
    p_event.Skip();
    return;
  }

  const wxSheetCoords coords = p_event.GetCoords();
  m_table->DeleteRowHeaderStrategy(coords.GetCol(), coords.GetRow());
}

wxString RowPlayerWidget::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsLabelCell(p_coords)) {
    return wxT("");
  }

  if (m_table->GetRowHeaderColCount() == 0) {
    return wxT("Payoffs");
  }

  const int player = m_table->GetRowHeaderPlayer(p_coords.GetCol());
  const int strat = m_table->GetRowHeaderStrategy(p_coords.GetCol(), p_coords.GetRow());

  return {m_table->GetStrategyByPlayerAndIndex(player, strat)->GetLabel().c_str(), *wxConvCurrent};
}

void RowPlayerWidget::SetCellValue(const wxSheetCoords &p_coords, const wxString &p_value)
{
  m_table->RenameRowHeaderStrategy(p_coords.GetCol(), p_coords.GetRow(), p_value);
}

wxSheetCellAttr RowPlayerWidget::GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const
{
  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  if (m_table->GetRowHeaderColCount() > 0) {
    attr.SetForegroundColour(
        m_table->GetPlayerColor(m_table->GetRowHeaderPlayer(p_coords.GetCol())));
    attr.SetReadOnly(m_table->IsReadOnly());
  }
  else {
    attr.SetForegroundColour(*wxBLACK);
    attr.SetReadOnly(true);
  }
  attr.SetBackgroundColour(*wxLIGHT_GREY);
  return attr;
}

void RowPlayerWidget::DrawCell(wxDC &p_dc, const wxSheetCoords &p_coords)
{
  TableWidgetBase::DrawCell(p_dc, p_coords);

  if (!m_table->ShowDominance() || IsLabelCell(p_coords) || m_table->GetRowHeaderColCount() == 0) {
    return;
  }

  if (m_table->IsRowHeaderStrategyDominated(p_coords.GetCol(), p_coords.GetRow(), false)) {
    const int player = m_table->GetRowHeaderPlayer(p_coords.GetCol());
    const wxRect rect = CellToRect(p_coords);

    if (m_table->IsRowHeaderStrategyDominated(p_coords.GetCol(), p_coords.GetRow(), true)) {
      p_dc.SetPen(wxPen(m_table->GetPlayerColor(player), 2, wxPENSTYLE_SOLID));
    }
    else {
      p_dc.SetPen(wxPen(m_table->GetPlayerColor(player), 1, wxPENSTYLE_SHORT_DASH));
    }

    p_dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    p_dc.DrawLine(rect.x + rect.width, rect.y, rect.x, rect.y + rect.height);
  }
}

void RowPlayerWidget::OnUpdate()
{
  const int newRows = m_table->GetRowHeaderRowCount();
  if (newRows > GetNumberRows()) {
    InsertRows(0, newRows - GetNumberRows());
  }
  if (newRows < GetNumberRows()) {
    DeleteRows(0, GetNumberRows() - newRows);
  }

  const int newCols = m_table->GetRowHeaderColCount();
  if (newCols > GetNumberCols()) {
    InsertCols(0, newCols - GetNumberCols());
  }
  if (newCols < GetNumberCols()) {
    DeleteCols(0, GetNumberCols() - newCols);
  }
  if (newCols == 0) {
    InsertCols(0, 1);
  }

  for (int col = 0; col < GetNumberCols(); col++) {
    for (int row = 0; row < GetNumberRows();
         SetCellSpan(wxSheetCoords(row++, col), wxSheetCoords(1, 1)))
      ;

    const int span = m_table->GetRowHeaderRowSpan(col);

    int row = 0;
    while (row < GetNumberRows()) {
      SetCellSpan(wxSheetCoords(row, col), wxSheetCoords(span, 1));
      row += span;
    }
  }

  Refresh();
}

bool RowPlayerWidget::DropText(wxCoord p_x, wxCoord p_y, const wxString &p_text)
{
  if (p_text[0] == 'P') {
    long pl;
    p_text.Right(p_text.Length() - 1).ToLong(&pl);

    if (m_table->NumRowPlayers() == 0) {
      m_table->SetRowPlayer(1, pl);
      return true;
    }

    for (int col = 0; col < GetNumberCols(); col++) {
      const wxRect rect = CellToRect(wxSheetCoords(0, col));

      if (p_x >= rect.x && p_x < rect.x + rect.width / 2) {
        m_table->SetRowPlayer(col + 1, pl);
        return true;
      }
      else if (p_x >= rect.x + rect.width / 2 && p_x < rect.x + rect.width) {
        m_table->SetRowPlayer(col + 2, pl);
        return true;
      }
    }
  }

  return false;
}

//=========================================================================
//                       class ColPlayerWidget
//=========================================================================

class ColPlayerWidget final : public TableWidgetBase {
  TableWidget *m_table;

  /// @name Overriding wxSheet members for data access
  //@{
  /// Returns the value in the cell
  wxString GetCellValue(const wxSheetCoords &) override;
  /// Sets the value in the cell, by relabeling the strategy
  void SetCellValue(const wxSheetCoords &, const wxString &) override;
  /// Returns the attributes of the cell
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const override;
  //@}

  /// @name Overriding wxSheet members to customize drawing
  //@{
  /// Overrides to draw dominance indicators
  void DrawCell(wxDC &p_dc, const wxSheetCoords &p_coords) override;
  //@}

  void OnCellRightClick(wxSheetEvent &);

public:
  /// @name Lifecycle
  //@{
  /// Constructor
  ColPlayerWidget(TableWidget *p_parent);
  //@}

  /// @name Synchronizing with document state
  //@{
  void OnUpdate();
  //@}

  /// @name Drop target interaction
  //@{
  /// Called when the drop target receives text.
  bool DropText(int p_x, int p_y, const wxString &p_text) override;
  //@}
};

ColPlayerWidget::ColPlayerWidget(TableWidget *p_parent)
  : TableWidgetBase(p_parent, wxID_ANY), m_table(p_parent)
{
  CreateGrid(m_table->NumColPlayers(), 0);
  SetRowLabelWidth(1);
  SetColLabelHeight(1);
  SetScrollBarMode(SB_NEVER);
  SetGridLineColour(*wxBLACK);
  wxWindow::SetBackgroundColour(*wxLIGHT_GREY);

  wxWindow::SetDropTarget(new gbtTableWidgetDropTarget(this));

  Connect(GetId(), wxEVT_SHEET_CELL_RIGHT_DOWN,
          reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction, wxSheetEventFunction(&ColPlayerWidget::OnCellRightClick))));
}

void ColPlayerWidget::OnCellRightClick(wxSheetEvent &p_event)
{
  if (m_table->GetColHeaderRowCount() == 0 || m_table->IsReadOnly()) {
    p_event.Skip();
    return;
  }

  const wxSheetCoords coords = p_event.GetCoords();
  m_table->DeleteColHeaderStrategy(coords.GetRow(), coords.GetCol());
}

void ColPlayerWidget::OnUpdate()
{
  const int newCols = m_table->GetColHeaderColCount();
  if (newCols > GetNumberCols()) {
    InsertCols(0, newCols - GetNumberCols());
  }
  if (newCols < GetNumberCols()) {
    DeleteCols(0, GetNumberCols() - newCols);
  }

  const int newRows = m_table->GetColHeaderRowCount();
  if (newRows > GetNumberRows()) {
    InsertRows(0, newRows - GetNumberRows());
  }
  if (newRows < GetNumberRows()) {
    DeleteRows(0, GetNumberRows() - newRows);
  }
  if (newRows == 0) {
    InsertRows(0, 1);
  }

  for (int row = 0; row < GetNumberRows(); row++) {
    for (int col = 0; col < GetNumberCols();
         SetCellSpan(wxSheetCoords(row, col++), wxSheetCoords(1, 1)))
      ;

    const int span = m_table->GetColHeaderColSpan(row);

    int col = 0;
    while (col < GetNumberCols()) {
      SetCellSpan(wxSheetCoords(row, col), wxSheetCoords(1, span));
      col += span;
    }
  }

  Refresh();
}

wxString ColPlayerWidget::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsLabelCell(p_coords)) {
    return wxT("");
  }

  if (m_table->GetColHeaderRowCount() == 0) {
    return wxT("Payoffs");
  }

  const int player = m_table->GetColHeaderPlayer(p_coords.GetRow());
  const int strat = m_table->GetColHeaderStrategy(p_coords.GetRow(), p_coords.GetCol());

  return {m_table->GetStrategyByPlayerAndIndex(player, strat)->GetLabel().c_str(), *wxConvCurrent};
}

void ColPlayerWidget::SetCellValue(const wxSheetCoords &p_coords, const wxString &p_value)
{
  m_table->RenameColHeaderStrategy(p_coords.GetRow(), p_coords.GetCol(), p_value);
}

wxSheetCellAttr ColPlayerWidget::GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const
{
  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  if (m_table->GetColHeaderRowCount() > 0) {
    attr.SetForegroundColour(
        m_table->GetPlayerColor(m_table->GetColHeaderPlayer(p_coords.GetRow())));
    attr.SetReadOnly(m_table->IsReadOnly());
  }
  else {
    attr.SetForegroundColour(*wxBLACK);
    attr.SetReadOnly(true);
  }
  attr.SetBackgroundColour(*wxLIGHT_GREY);
  return attr;
}

void ColPlayerWidget::DrawCell(wxDC &p_dc, const wxSheetCoords &p_coords)
{
  TableWidgetBase::DrawCell(p_dc, p_coords);

  if (!m_table->ShowDominance() || IsLabelCell(p_coords) || m_table->GetColHeaderRowCount() == 0) {
    return;
  }

  if (m_table->IsColHeaderStrategyDominated(p_coords.GetRow(), p_coords.GetCol(), false)) {
    const int player = m_table->GetColHeaderPlayer(p_coords.GetRow());
    const wxRect rect = CellToRect(p_coords);

    if (m_table->IsColHeaderStrategyDominated(p_coords.GetRow(), p_coords.GetCol(), true)) {
      p_dc.SetPen(wxPen(m_table->GetPlayerColor(player), 2, wxPENSTYLE_SOLID));
    }
    else {
      p_dc.SetPen(wxPen(m_table->GetPlayerColor(player), 1, wxPENSTYLE_SHORT_DASH));
    }

    p_dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    p_dc.DrawLine(rect.x + rect.width, rect.y, rect.x, rect.y + rect.height);
  }
}

bool ColPlayerWidget::DropText(wxCoord p_x, wxCoord p_y, const wxString &p_text)
{
  if (p_text[0] == 'P') {
    long pl;
    p_text.Right(p_text.Length() - 1).ToLong(&pl);

    if (m_table->NumColPlayers() == 0) {
      m_table->SetColPlayer(1, pl);
      return true;
    }

    for (int row = 0; row < GetNumberRows(); row++) {
      const wxRect rect = CellToRect(wxSheetCoords(row, 0));

      if (p_y >= rect.y && p_y < rect.y + rect.height / 2) {
        m_table->SetColPlayer(row + 1, pl);
        return true;
      }
      else if (p_y >= rect.y + rect.height / 2 && p_y < rect.y + rect.height) {
        m_table->SetColPlayer(row + 2, pl);
        return true;
      }
    }
  }

  return false;
}

//=========================================================================
//                       class PayoffsWidget
//=========================================================================

class PayoffsWidget : public TableWidgetBase {
  TableWidget *m_table;

  /// @name Overriding wxSheet members for data access
  //@{
  /// Returns the value in the cell
  wxString GetCellValue(const wxSheetCoords &) override;
  /// Sets the value in the cell, by editing the outcome
  void SetCellValue(const wxSheetCoords &, const wxString &) override;
  /// Returns the attributes of the cell
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const override;
  //@}

  /// @name Overriding wxSheet members for drawing behavior
  //@{
  /// Draws dark borders between contingencies
  void DrawCellBorder(wxDC &p_dc, const wxSheetCoords &p_coords) override;
  /// Overrides to draw dominance indicators
  void DrawCell(wxDC &p_dc, const wxSheetCoords &p_coords) override;
  //@}

  /// @name Overriding wxSheet members for event-handling behavior
  //@{
  /// Implement custom tab-traversal behavior
  void OnKeyDown(wxKeyEvent &);
  //@}

  /// Maps columns to corresponding player
  int ColToPlayer(int p_col) const;

public:
  PayoffsWidget(TableWidget *p_parent);

  /// @name Synchronizing with document state
  //@{
  void OnUpdate();
  //@}

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(PayoffsWidget, TableWidgetBase)
EVT_KEY_DOWN(PayoffsWidget::OnKeyDown)
END_EVENT_TABLE()

PayoffsWidget::PayoffsWidget(TableWidget *p_parent)
  : TableWidgetBase(p_parent, wxID_ANY), m_table(p_parent)
{
  CreateGrid(0, 0);
  SetRowLabelWidth(1);
  SetColLabelHeight(1);
}

//
// Payoffs are ordered first by row players (in hierarchical order),
// followed by column players (in hierarchical order)
//
int PayoffsWidget::ColToPlayer(int p_col) const
{
  return m_table->GetPayoffPlayerForColumn(p_col);
}

void PayoffsWidget::OnUpdate()
{
  const int newCols = m_table->GetPayoffColCount();
  if (newCols > GetNumberCols()) {
    InsertCols(0, newCols - GetNumberCols());
  }
  if (newCols < GetNumberCols()) {
    DeleteCols(0, GetNumberCols() - newCols);
  }

  const int newRows = m_table->GetPayoffRowCount();
  if (newRows > GetNumberRows()) {
    InsertRows(0, newRows - GetNumberRows());
  }
  if (newRows < GetNumberRows()) {
    DeleteRows(0, GetNumberRows() - newRows);
  }

  Refresh();
}

bool TableWidget::IsReadOnly() const { return m_doc->IsTree(); }

wxColour TableWidget::GetPlayerColor(int player) const
{
  return m_doc->GetStyle().GetPlayerColor(player);
}

bool TableWidget::IsRowHeaderStrategyDominated(int headerCol, int headerRow, bool strict) const
{
  const StrategySupportProfile &support = GetSupport();
  const int player = GetRowHeaderPlayer(headerCol);
  const int strat = GetRowHeaderStrategy(headerCol, headerRow);
  return support.IsDominated(GetStrategyByPlayerAndIndex(player, strat), strict);
}

bool TableWidget::IsColHeaderStrategyDominated(int headerRow, int headerCol, bool strict) const
{
  const StrategySupportProfile &support = GetSupport();
  const int player = GetColHeaderPlayer(headerRow);
  const int strat = GetColHeaderStrategy(headerRow, headerCol);
  return support.IsDominated(GetStrategyByPlayerAndIndex(player, strat), strict);
}

wxString PayoffsWidget::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsLabelCell(p_coords)) {
    return wxT("");
  }

  const PureStrategyProfile profile = m_table->GetPayoffProfile(p_coords);
  auto player = m_table->GetPayoffPlayer(ColToPlayer(p_coords.GetCol()));
  return {lexical_cast<std::string>(profile->GetPayoff(player)).c_str(), *wxConvCurrent};
}

void PayoffsWidget::SetCellValue(const wxSheetCoords &p_coords, const wxString &p_value)
{
  m_table->SetPayoffCellValue(p_coords, p_value);
}

wxSheetCellAttr PayoffsWidget::GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const
{
  if (IsLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultColLabelAttr);
    attr.SetBackgroundColour(*wxLIGHT_GREY);
    return attr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  const int player = ColToPlayer(p_coords.GetCol());
  attr.SetForegroundColour(m_table->GetPlayerColor(player));
  attr.SetRenderer(wxSheetCellRenderer(new RationalRendererRefData()));
  attr.SetEditor(wxSheetCellEditor(new RationalEditorRefData()));
  attr.SetReadOnly(m_table->IsReadOnly());
  return attr;
}

void PayoffsWidget::DrawCellBorder(wxDC &p_dc, const wxSheetCoords &p_coords)
{
  TableWidgetBase::DrawCellBorder(p_dc, p_coords);

  const wxRect rect(CellToRect(p_coords));
  if (rect.width < 1 || rect.height < 1) {
    return;
  }

  p_dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));

  // Draw the dark border to the right of the last column of a contingency
  if ((p_coords.GetCol() + 1) % m_table->GetPayoffColumnsPerContingency() == 0) {
    p_dc.DrawLine(rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height + 1);
  }

  // Draw the bottom border -- currently always dark
  p_dc.DrawLine(rect.x - 1, rect.y + rect.height, rect.x + rect.width, rect.y + rect.height);

  // Draw the top border for the first row
  if (p_coords.GetRow() == 0) {
    p_dc.DrawLine(rect.x - 1, rect.y, rect.x + rect.width, rect.y);
  }
}

void PayoffsWidget::DrawCell(wxDC &p_dc, const wxSheetCoords &p_coords)
{
  TableWidgetBase::DrawCell(p_dc, p_coords);

  if (!m_table->ShowDominance() || IsLabelCell(p_coords)) {
    return;
  }

  auto player = m_table->GetPayoffPlayer(p_coords.GetCol());

  if (m_table->IsPayoffStrategyDominated(p_coords, false)) {
    const wxRect rect = CellToRect(p_coords);
    if (m_table->IsPayoffStrategyDominated(p_coords, true)) {
      p_dc.SetPen(wxPen(m_table->GetPlayerColor(player->GetNumber()), 2, wxPENSTYLE_SOLID));
    }
    else {
      p_dc.SetPen(wxPen(m_table->GetPlayerColor(player->GetNumber()), 1, wxPENSTYLE_SHORT_DASH));
    }
    p_dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    p_dc.DrawLine(rect.x + rect.width, rect.y, rect.x, rect.y + rect.height);
  }
}

//!
//! Overriding default wxSheet behavior: when editing, accepting the
//! edited value via the TAB key automatically moves the cursor to
//! the right *and* creates the editor in the next cell.  In addition,
//! tabbing off the rightmost cell entry automatically "wraps" to the
//! next row.
//!
void PayoffsWidget::OnKeyDown(wxKeyEvent &p_event)
{
  if (GetNumberRows() && GetNumberCols()) {
    switch (p_event.GetKeyCode()) {
    case WXK_TAB: {
      if (IsCellEditControlCreated()) {
        DisableCellEditControl(true);

        int newRow = GetGridCursorRow(), newCol = GetGridCursorCol();

        if (p_event.ShiftDown()) {
          newCol--;
          if (newCol < 0) {
            newCol = GetNumberCols() - 1;
            newRow--;
            if (newRow < 0) {
              newRow = GetNumberRows() - 1;
            }
          }
        }
        else {
          newCol++;
          if (newCol >= GetNumberCols()) {
            newCol = 0;
            newRow++;
            if (newRow >= GetNumberRows()) {
              newRow = 0;
            }
          }
        }
        SetGridCursorCell(wxSheetCoords(newRow, newCol));
        MakeCellVisible(GetGridCursorCell());
        EnableCellEditControl(GetGridCursorCell());
      }
      break;
    }
    default:
      p_event.Skip();
    }
  }
}

//=========================================================================
//                       TableWidget: Lifecycle
//=========================================================================

TableWidget::TableWidget(NfgPanel *p_parent, wxWindowID p_id, GameDocument *p_doc)
  : wxPanel(p_parent, p_id), m_doc(p_doc), m_nfgPanel(p_parent), m_payoffSheet(nullptr),
    m_rowSheet(nullptr), m_colSheet(nullptr),
    m_layout(std::make_shared<StrategicTableLayout>(p_doc))
{
  // These depend on the row and column player lists having been populated,
  // which suggests some refactoring ought to be done as to where/how those
  // row and column players are recorded
  // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)
  m_payoffSheet = new PayoffsWidget(this);
  m_rowSheet = new RowPlayerWidget(this);
  m_colSheet = new ColPlayerWidget(this);
  // NOLINTEND(cppcoreguidelines-prefer-member-initializer)
  m_payoffSheet->SetGridLineColour(*wxWHITE);

  auto *topSizer = new wxFlexGridSizer(2, 2, 0, 0);
  topSizer->AddGrowableRow(1);
  topSizer->AddGrowableCol(1);
  topSizer->Add(new wxPanel(this, wxID_ANY));
  topSizer->Add(m_colSheet, 1, wxEXPAND, 0);
  topSizer->Add(m_rowSheet, 1, wxEXPAND, 0);
  topSizer->Add(m_payoffSheet, 1, wxEXPAND, 0);

  SetSizer(topSizer);
  wxWindowBase::Layout();

  Connect(m_rowSheet->GetId(), wxEVT_SHEET_VIEW_CHANGED,
          reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction,
              static_cast<wxSheetEventFunction>(&TableWidget::OnRowSheetScroll))));

  Connect(m_colSheet->GetId(), wxEVT_SHEET_VIEW_CHANGED,
          reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction,
              static_cast<wxSheetEventFunction>(&TableWidget::OnColSheetScroll))));

  Connect(
      m_payoffSheet->GetId(), wxEVT_SHEET_VIEW_CHANGED,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnPayoffScroll))));

  Connect(
      m_rowSheet->GetId(), wxEVT_SHEET_ROW_SIZE,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnRowSheetRow))));

  Connect(
      m_payoffSheet->GetId(), wxEVT_SHEET_ROW_SIZE,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnPayoffRow))));

  Connect(m_colSheet->GetId(), wxEVT_SHEET_COL_SIZE,
          reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction,
              static_cast<wxSheetEventFunction>(&TableWidget::OnColSheetColumn))));

  Connect(
      m_payoffSheet->GetId(), wxEVT_SHEET_COL_SIZE,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnPayoffColumn))));

  Connect(m_rowSheet->GetId(), wxEVT_SHEET_COL_SIZE,
          reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction,
              static_cast<wxSheetEventFunction>(&TableWidget::OnRowSheetColumn))));

  Connect(
      m_colSheet->GetId(), wxEVT_SHEET_ROW_SIZE,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnColSheetRow))));

  Connect(
      m_rowSheet->GetId(), wxEVT_SHEET_EDITOR_ENABLED,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnBeginEdit))));
  Connect(
      m_colSheet->GetId(), wxEVT_SHEET_EDITOR_ENABLED,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnBeginEdit))));
  Connect(
      m_payoffSheet->GetId(), wxEVT_SHEET_EDITOR_ENABLED,
      reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
          wxSheetEventFunction, static_cast<wxSheetEventFunction>(&TableWidget::OnBeginEdit))));
}

int TableWidget::GetRowPaneWidth() const
{
  if (!m_rowSheet || m_rowSheet->GetNumberCols() == 0 || m_rowSheet->GetNumberRows() == 0) {
    return 0;
  }

  return m_rowSheet->CellToRect(wxSheetCoords(0, m_rowSheet->GetNumberCols() - 1)).GetRight();
}

int TableWidget::GetColPaneHeight() const
{
  if (!m_colSheet || m_colSheet->GetNumberRows() == 0 || m_colSheet->GetNumberCols() == 0) {
    return 0;
  }

  return m_colSheet->CellToRect(wxSheetCoords(m_colSheet->GetNumberRows() - 1, 0)).GetBottom();
}

void TableWidget::UpdateLabelPanelSizes()
{
  m_rowSheet->SetMinSize(wxSize(GetRowPaneWidth(), -1));
  m_colSheet->SetMinSize(wxSize(-1, GetColPaneHeight()));

  m_rowSheet->InvalidateBestSize();
  m_colSheet->InvalidateBestSize();
}

//!
//! Scroll row and column label sheets to match payoff sheet origin.
//! Note that wxSheet uses coordinates of -1 to indicate no scroll.
//!
//@{
void TableWidget::OnRowSheetScroll(wxSheetEvent &)
{
  m_payoffSheet->SetGridOrigin(-1, m_rowSheet->GetGridOrigin().y);
}

void TableWidget::OnColSheetScroll(wxSheetEvent &)
{
  m_payoffSheet->SetGridOrigin(m_colSheet->GetGridOrigin().x, -1);
}

void TableWidget::OnPayoffScroll(wxSheetEvent &)
{
  m_colSheet->SetGridOrigin(m_payoffSheet->GetGridOrigin().x, 0);
  m_rowSheet->SetGridOrigin(0, m_payoffSheet->GetGridOrigin().y);
}
//@}

//!
//! These keep the row heights synchronized
//!
//@{
void TableWidget::OnRowSheetRow(wxSheetEvent &p_event)
{
  const int height = m_rowSheet->GetRowHeight(p_event.GetRow());
  m_payoffSheet->SetDefaultRowHeight(height, true);
  m_payoffSheet->AdjustScrollbars();
  m_payoffSheet->Refresh();
  m_rowSheet->SetDefaultRowHeight(height, true);
  m_rowSheet->Refresh();
}

void TableWidget::OnPayoffRow(wxSheetEvent &p_event)
{
  const int height = m_payoffSheet->GetRowHeight(p_event.GetRow());
  m_payoffSheet->SetDefaultRowHeight(height, true);
  m_payoffSheet->AdjustScrollbars();
  m_payoffSheet->Refresh();
  m_rowSheet->SetDefaultRowHeight(height, true);
  m_rowSheet->Refresh();
}
//@}

//!
//! These keep the column widths synchronized
//!
//@{
void TableWidget::OnColSheetColumn(wxSheetEvent &p_event)
{
  const int width = m_colSheet->GetColWidth(p_event.GetCol());
  m_payoffSheet->SetDefaultColWidth(width, true);
  m_payoffSheet->AdjustScrollbars();
  m_payoffSheet->Refresh();
  m_colSheet->SetDefaultColWidth(width, true);
  m_colSheet->Refresh();
}

void TableWidget::OnPayoffColumn(wxSheetEvent &p_event)
{
  const int width = m_payoffSheet->GetColWidth(p_event.GetCol());
  m_payoffSheet->SetDefaultColWidth(width, true);
  m_payoffSheet->AdjustScrollbars();
  m_payoffSheet->Refresh();
  m_colSheet->SetDefaultColWidth(width, true);
  m_colSheet->Refresh();
}
//@}

//!
//! These handle correctly sizing the label windows
//!
//@{
void TableWidget::OnRowSheetColumn(wxSheetEvent &p_event)
{
  m_rowSheet->SetDefaultColWidth(m_rowSheet->GetColWidth(p_event.GetCol()), true);
  GetSizer()->Layout();
}

void TableWidget::OnColSheetRow(wxSheetEvent &p_event)
{
  m_colSheet->SetDefaultRowHeight(m_colSheet->GetRowHeight(p_event.GetRow()), true);
  GetSizer()->Layout();
}
//@}

//!
//! This alerts the document to have any other windows post their pending
//! edits.
//!
void TableWidget::OnBeginEdit(wxSheetEvent &) { m_doc->PostPendingChanges(); }

void TableWidget::ReconcilePlayers() { m_layout->ReconcilePlayers(); }

void TableWidget::UpdatePayoffPanel() { dynamic_cast<PayoffsWidget *>(m_payoffSheet)->OnUpdate(); }

void TableWidget::UpdateLabelPanelMargins()
{
  // We add margins to the player labels to match the scrollbars,
  // so scrolling matches up
  auto payoffs = dynamic_cast<PayoffsWidget *>(m_payoffSheet);
  m_colSheet->SetMargins(payoffs->GetVerticalScrollBar()->GetSize().GetWidth(), -1);
  ;
  m_rowSheet->SetMargins(-1, payoffs->GetHorizontalScrollBar()->GetSize().GetHeight());
}

void TableWidget::UpdateLabelPanels()
{
  dynamic_cast<RowPlayerWidget *>(m_rowSheet)->OnUpdate();
  dynamic_cast<ColPlayerWidget *>(m_colSheet)->OnUpdate();
}

void TableWidget::OnUpdate()
{
  ReconcilePlayers();
  UpdatePayoffPanel();
  UpdateLabelPanelMargins();
  UpdateLabelPanels();
  UpdateLabelPanelSizes();
  Layout();
}

void TableWidget::PostPendingChanges()
{
  if (m_payoffSheet->IsCellEditControlShown()) {
    m_payoffSheet->DisableCellEditControl(true);
  }

  if (m_rowSheet->IsCellEditControlShown()) {
    m_rowSheet->DisableCellEditControl(true);
  }

  if (m_colSheet->IsCellEditControlShown()) {
    m_colSheet->DisableCellEditControl(true);
  }
}

bool TableWidget::ShowDominance() const { return m_nfgPanel->IsDominanceShown(); }

//=========================================================================
//                      TableWidget: View state
//=========================================================================

void TableWidget::SetRowPlayer(int index, int pl)
{
  m_layout->SetRowPlayer(index, pl);
  OnUpdate();
}

void TableWidget::SetColPlayer(int index, int pl)
{
  m_layout->SetColPlayer(index, pl);
  OnUpdate();
}

class gbtNfgPrintout : public wxPrintout {
private:
  TableWidget *m_table;

public:
  gbtNfgPrintout(TableWidget *p_table, const wxString &p_label)
    : wxPrintout(p_label), m_table(p_table)
  {
  }
  ~gbtNfgPrintout() override = default;

  bool OnPrintPage(int) override
  {
    m_table->RenderGame(*GetDC(), 50, 50);
    return true;
  }
  bool HasPage(int page) override { return (page <= 1); }
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) override
  {
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
  }
};

wxPrintout *TableWidget::GetPrintout()
{
  return new gbtNfgPrintout(this, wxString(m_doc->GetGame()->GetTitle().c_str(), *wxConvCurrent));
}

bool TableWidget::GetBitmap(wxBitmap &p_bitmap, int p_marginX, int p_marginY)
{
  const int width =
      (m_rowSheet->CellToRect(wxSheetCoords(0, m_rowSheet->GetNumberCols() - 1)).GetRight() +
       m_colSheet->CellToRect(wxSheetCoords(0, m_colSheet->GetNumberCols() - 1)).GetRight() +
       2 * p_marginX);
  const int height =
      (m_rowSheet->CellToRect(wxSheetCoords(m_rowSheet->GetNumberRows() - 1, 0)).GetBottom() +
       m_colSheet->CellToRect(wxSheetCoords(m_colSheet->GetNumberRows() - 1, 0)).GetBottom() +
       2 * p_marginY);

  if (width > 65000 || height > 65000) {
    // This is just too huge to export to graphics
    return false;
  }

  wxMemoryDC dc;
  p_bitmap = wxBitmap(width, height);
  dc.SelectObject(p_bitmap);
  dc.Clear();
  RenderGame(dc, p_marginX, p_marginY);
  return true;
}

void TableWidget::GetSVG(const wxString &p_filename, int p_marginX, int p_marginY)
{
  const int width =
      (m_rowSheet->CellToRect(wxSheetCoords(0, m_rowSheet->GetNumberCols() - 1)).GetRight() +
       m_colSheet->CellToRect(wxSheetCoords(0, m_colSheet->GetNumberCols() - 1)).GetRight() +
       2 * p_marginX);
  const int height =
      (m_rowSheet->CellToRect(wxSheetCoords(m_rowSheet->GetNumberRows() - 1, 0)).GetBottom() +
       m_colSheet->CellToRect(wxSheetCoords(m_colSheet->GetNumberRows() - 1, 0)).GetBottom() +
       2 * p_marginY);

  wxSVGFileDC dc(p_filename, width, height);
  // For some reason, this needs to be initialized
  dc.SetLogicalScale(1.0, 1.0);
  RenderGame(dc, p_marginX, p_marginY);
}

void TableWidget::RenderGame(wxDC &p_dc, int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  const int maxX =
      (m_rowSheet->CellToRect(wxSheetCoords(0, m_rowSheet->GetNumberCols() - 1)).GetRight() +
       m_colSheet->CellToRect(wxSheetCoords(0, m_colSheet->GetNumberCols() - 1)).GetRight());
  const int maxY =
      (m_rowSheet->CellToRect(wxSheetCoords(m_rowSheet->GetNumberRows() - 1, 0)).GetBottom() +
       m_colSheet->CellToRect(wxSheetCoords(m_colSheet->GetNumberRows() - 1, 0)).GetBottom());

  // Get the size of the DC in pixels
  wxCoord w, h;
  p_dc.GetSize(&w, &h);

  // Calculate a scaling factor
  const double scaleX = static_cast<double>(w) / static_cast<double>(maxX + 2 * p_marginX);
  const double scaleY = static_cast<double>(h) / static_cast<double>(maxY + 2 * p_marginY);
  const double scale = (scaleX < scaleY) ? scaleX : scaleY;
  // Here, zooming in is often a good idea, since the number of pixels
  // on a page is generally quite large
  p_dc.SetUserScale(scale, scale);

  // Calculate the position on the DC to center the tree
  auto posX = (double)((w - (maxX * scale)) / 2.0);
  auto posY = (double)((h - (maxY * scale)) / 2.0);

  // The X and Y coordinates of the upper left of the payoff table
  const int payoffX = static_cast<int>(
      m_rowSheet->CellToRect(wxSheetCoords(0, m_rowSheet->GetNumberCols() - 1)).GetRight() *
      scale);
  const int payoffY = static_cast<int>(
      m_colSheet->CellToRect(wxSheetCoords(m_colSheet->GetNumberRows() - 1, 0)).GetBottom() *
      scale);

  p_dc.SetDeviceOrigin(static_cast<int>(posX), payoffY + static_cast<int>(posY));
  m_rowSheet->DrawGridCells(
      p_dc, wxSheetBlock(0, 0, m_rowSheet->GetNumberRows(), m_rowSheet->GetNumberCols()));

  p_dc.SetDeviceOrigin(payoffX + static_cast<int>(posX), static_cast<int>(posY));
  m_colSheet->DrawGridCells(
      p_dc, wxSheetBlock(0, 0, m_colSheet->GetNumberRows(), m_colSheet->GetNumberCols()));

  p_dc.SetDeviceOrigin(payoffX + static_cast<int>(posX), payoffY + static_cast<int>(posY));
  m_payoffSheet->DrawGridCells(
      p_dc, wxSheetBlock(0, 0, m_payoffSheet->GetNumberRows(), m_payoffSheet->GetNumberCols()));
}

void TableWidget::RenameRowHeaderStrategy(int headerCol, int headerRow, const wxString &value)
{
  const int player = GetRowHeaderPlayer(headerCol);
  const int strat = GetRowHeaderStrategy(headerCol, headerRow);

  m_doc->DoSetStrategyLabel(GetStrategyByPlayerAndIndex(player, strat), value);
}

void TableWidget::RenameColHeaderStrategy(int headerRow, int headerCol, const wxString &value)
{
  const int player = GetColHeaderPlayer(headerRow);
  const int strat = GetColHeaderStrategy(headerRow, headerCol);

  m_doc->DoSetStrategyLabel(GetStrategyByPlayerAndIndex(player, strat), value);
}

void TableWidget::DeleteRowHeaderStrategy(int headerCol, int headerRow)
{
  const int player = GetRowHeaderPlayer(headerCol);
  const int strat = GetRowHeaderStrategy(headerCol, headerRow);

  m_doc->DoDeleteStrategy(GetStrategyByPlayerAndIndex(player, strat));
}

void TableWidget::DeleteColHeaderStrategy(int headerRow, int headerCol)
{
  const int player = GetColHeaderPlayer(headerRow);
  const int strat = GetColHeaderStrategy(headerRow, headerCol);

  m_doc->DoDeleteStrategy(GetStrategyByPlayerAndIndex(player, strat));
}

void TableWidget::SetPayoffCellValue(const wxSheetCoords &coords, const wxString &value)
{
  PureStrategyProfile profile = GetPayoffProfile(coords);
  GameOutcome outcome = profile->GetOutcome();
  if (!outcome) {
    m_doc->DoNewOutcome(profile);
    profile = GetPayoffProfile(coords);
    outcome = profile->GetOutcome();
  }

  const int player = GetPayoffPlayerForColumn(coords.GetCol());

  try {
    m_doc->DoSetPayoff(outcome, player, value);
  }
  catch (ValueException &) {
    // For the moment, we will just silently discard edits which
    // give payoffs that are not valid numbers
    return;
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

GamePlayer TableWidget::GetPayoffPlayer(int payoffCol) const
{
  return m_doc->GetGame()->GetPlayer(GetPayoffPlayerForColumn(payoffCol));
}

int TableWidget::GetPayoffColumnsPerContingency() const { return m_doc->GetGame()->NumPlayers(); }

bool TableWidget::IsPayoffStrategyDominated(const wxSheetCoords &coords, bool strict) const
{
  const PureStrategyProfile profile = GetPayoffProfile(coords);
  auto player = GetPayoffPlayer(coords.GetCol());
  return GetSupport().IsDominated(profile->GetStrategy(player), strict);
}

GameStrategy TableWidget::GetStrategyByPlayerAndIndex(int player, int strategy) const
{
  auto strategies = GetSupport().GetStrategies(GetSupport().GetGame()->GetPlayer(player));
  return *std::next(strategies.begin(), strategy - 1);
}

} // namespace Gambit::GUI
