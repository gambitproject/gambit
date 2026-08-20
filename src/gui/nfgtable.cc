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
#include <wx/print.h> // for printing support
#include <wx/dcsvg.h> // for SVG output

#include <wx/grid.h>

#include "gamedoc.h"
#include "nfgpanel.h"
#include "nfgtable.h"
#include "dleditstrategies.h"
#include "dlexcept.h"

namespace Gambit::GUI {

//=========================================================================
//                helper: draw a whole grid to an arbitrary DC
//=========================================================================

//!
//! wxGrid has no direct equivalent of wxSheet's DrawGridCells(dc, block);
//! this walks the visible cells and asks each one's renderer to draw
//! itself at the appropriate device-DC rect. Used for print/bitmap/SVG
//! export, which render at an arbitrary scale/origin rather than to the
//! screen.
//!
static void DrawGridToDC(wxGrid *p_grid, wxDC &p_dc)
{
  for (int row = 0; row < p_grid->GetNumberRows(); row++) {
    for (int col = 0; col < p_grid->GetNumberCols(); col++) {
      int numRows, numCols;
      if (p_grid->GetCellSize(row, col, &numRows, &numCols) == wxGrid::CellSpan_Inside) {
        continue;
      }
      const wxRect rect = p_grid->CellToRect(row, col);
      const wxGridCellAttrPtr attr = p_grid->GetOrCreateCellAttrPtr(row, col);
      const wxGridCellRendererPtr renderer = attr->GetRendererPtr(p_grid, row, col);
      if (renderer) {
        renderer->Draw(*p_grid, *attr, p_dc, rect, row, col, false);
      }
    }
  }
}

//=========================================================================
//                      TableWidget: Lifecycle
//=========================================================================

bool TableWidget::IsReadOnly() const { return m_doc->GetGame()->IsTree(); }

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

TableWidget::TableWidget(NfgPanel *p_parent, wxWindowID p_id, GameDocument *p_doc)
  : wxPanel(p_parent, p_id), m_doc(p_doc), m_nfgPanel(p_parent), m_payoffGrid(nullptr),
    m_rowGrid(nullptr), m_colGrid(nullptr), m_layout(std::make_shared<StrategicTableLayout>(p_doc))
{
  // These depend on the row and column player lists having been populated,
  // which suggests some refactoring ought to be done as to where/how those
  // row and column players are recorded
  InitPayoffGrid();
  InitHeaderGrids();
  // Near-invisible default gridlines for separators within a contingency
  // (columns sharing the same strategy profile, one per player) -- the
  // player colour-coding on the text already carries that grouping, so the
  // line doesn't need to compete for attention. PayoffCellRenderer draws a
  // full-black line specifically at contingency/row boundaries, which is
  // the one structural division actually worth emphasizing.
  m_payoffGrid->SetGridLineColour(wxColour(238, 238, 238));

  // A touch more row height/breathing room reads as calmer than the tightly
  // packed default; the row header pane has to match since its rows are
  // the same logical rows as the payoff pane's.
  const int rowHeight = m_payoffGrid->GetDefaultRowSize() + 4;
  m_payoffGrid->SetDefaultRowSize(rowHeight, true);
  m_rowGrid->SetDefaultRowSize(rowHeight, true);

  auto *topSizer = new wxFlexGridSizer(2, 2, 0, 0);
  topSizer->AddGrowableRow(1);
  topSizer->AddGrowableCol(1);
  topSizer->Add(new wxPanel(this, wxID_ANY));
  topSizer->Add(m_colGrid, 1, wxEXPAND, 0);
  topSizer->Add(m_rowGrid, 1, wxEXPAND, 0);
  topSizer->Add(m_payoffGrid, 1, wxEXPAND, 0);

  SetSizer(topSizer);
  wxWindowBase::Layout();

  m_rowGrid->EnableScrolling(false, false);
  m_colGrid->EnableScrolling(false, false);

  //!
  //! Scroll sync: the payoff grid is the sole scroll master (header grids
  //! have scrolling disabled above), so we only need to listen here.
  //!
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_TOP, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_BOTTOM, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_LINEUP, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_LINEDOWN, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_PAGEUP, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_PAGEDOWN, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_THUMBTRACK, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &TableWidget::OnPayoffScroll, this);
  m_payoffGrid->Bind(wxEVT_MOUSEWHEEL, &TableWidget::OnPayoffMouseWheel, this);

  //!
  //! These keep the row heights synchronized
  //!
  m_rowGrid->Bind(wxEVT_GRID_ROW_SIZE, &TableWidget::OnRowGridRowSize, this);
  m_payoffGrid->Bind(wxEVT_GRID_ROW_SIZE, &TableWidget::OnPayoffGridRowSize, this);

  //!
  //! These keep the column widths synchronized
  //!
  m_colGrid->Bind(wxEVT_GRID_COL_SIZE, &TableWidget::OnColGridColSize, this);
  m_payoffGrid->Bind(wxEVT_GRID_COL_SIZE, &TableWidget::OnPayoffGridColSize, this);

  //!
  //! These handle correctly sizing the label windows
  //!
  m_rowGrid->Bind(wxEVT_GRID_COL_SIZE, &TableWidget::OnRowGridColSize, this);
  m_colGrid->Bind(wxEVT_GRID_ROW_SIZE, &TableWidget::OnColGridRowSize, this);

  m_rowGrid->Bind(wxEVT_GRID_EDITOR_SHOWN, &TableWidget::OnBeginEdit, this);
  m_colGrid->Bind(wxEVT_GRID_EDITOR_SHOWN, &TableWidget::OnBeginEdit, this);
  m_payoffGrid->Bind(wxEVT_GRID_EDITOR_SHOWN, &TableWidget::OnBeginEdit, this);
}

int TableWidget::GetRowPaneWidth() const
{
  if (!m_rowGrid || m_rowGrid->GetNumberCols() == 0 || m_rowGrid->GetNumberRows() == 0) {
    return 0;
  }

  return m_rowGrid->CellToRect(0, m_rowGrid->GetNumberCols() - 1).GetRight();
}

int TableWidget::GetColPaneHeight() const
{
  if (!m_colGrid || m_colGrid->GetNumberRows() == 0 || m_colGrid->GetNumberCols() == 0) {
    return 0;
  }

  return m_colGrid->CellToRect(m_colGrid->GetNumberRows() - 1, 0).GetBottom();
}

void TableWidget::UpdateLabelPanelSizes()
{
  m_rowGrid->SetMinSize(wxSize(GetRowPaneWidth(), -1));
  m_colGrid->SetMinSize(wxSize(-1, GetColPaneHeight()));

  m_rowGrid->InvalidateBestSize();
  m_colGrid->InvalidateBestSize();
}

void TableWidget::SyncScrollFromPayoff()
{
  int x, y;
  m_payoffGrid->GetViewStart(&x, &y);
  m_rowGrid->Scroll(0, y);
  m_colGrid->Scroll(x, 0);
}

void TableWidget::OnPayoffScroll(wxScrollWinEvent &p_event)
{
  p_event.Skip();
  CallAfter([this] { SyncScrollFromPayoff(); });
}

void TableWidget::OnPayoffMouseWheel(wxMouseEvent &p_event)
{
  p_event.Skip();
  CallAfter([this] { SyncScrollFromPayoff(); });
}

//!
//! These keep the row heights synchronized
//!
//@{
void TableWidget::OnRowGridRowSize(wxGridSizeEvent &p_event)
{
  const int height = m_rowGrid->GetRowSize(p_event.GetRowOrCol());
  m_payoffGrid->SetDefaultRowSize(height, true);
  m_payoffGrid->Refresh();
  m_rowGrid->SetDefaultRowSize(height, true);
  m_rowGrid->Refresh();
}

void TableWidget::OnPayoffGridRowSize(wxGridSizeEvent &p_event)
{
  const int height = m_payoffGrid->GetRowSize(p_event.GetRowOrCol());
  m_payoffGrid->SetDefaultRowSize(height, true);
  m_payoffGrid->Refresh();
  m_rowGrid->SetDefaultRowSize(height, true);
  m_rowGrid->Refresh();
}
//@}

//!
//! These keep the column widths synchronized
//!
//@{
void TableWidget::OnColGridColSize(wxGridSizeEvent &p_event)
{
  const int width = m_colGrid->GetColSize(p_event.GetRowOrCol());
  m_payoffGrid->SetDefaultColSize(width, true);
  m_payoffGrid->Refresh();
  m_colGrid->SetDefaultColSize(width, true);
  m_colGrid->Refresh();
}

void TableWidget::OnPayoffGridColSize(wxGridSizeEvent &p_event)
{
  const int width = m_payoffGrid->GetColSize(p_event.GetRowOrCol());
  m_payoffGrid->SetDefaultColSize(width, true);
  m_payoffGrid->Refresh();
  m_colGrid->SetDefaultColSize(width, true);
  m_colGrid->Refresh();
}
//@}

//!
//! These handle correctly sizing the label windows
//!
//@{
void TableWidget::OnRowGridColSize(wxGridSizeEvent &p_event)
{
  m_rowGrid->SetDefaultColSize(m_rowGrid->GetColSize(p_event.GetRowOrCol()), true);
  GetSizer()->Layout();
}

void TableWidget::OnColGridRowSize(wxGridSizeEvent &p_event)
{
  m_colGrid->SetDefaultRowSize(m_colGrid->GetRowSize(p_event.GetRowOrCol()), true);
  GetSizer()->Layout();
}
//@}

//!
//! This alerts the document to have any other windows post their pending
//! edits.
//!
void TableWidget::OnBeginEdit(wxGridEvent &) { m_doc->PostPendingChanges(); }

void TableWidget::ReconcilePlayers() { m_layout->ReconcilePlayers(); }

void TableWidget::UpdatePayoffPanel() { UpdatePayoffGrid(); }

void TableWidget::UpdateLabelPanelMargins()
{
  // NOTE: wxGrid has no direct equivalent of wxSheet's SetMargins (reserving
  // scrollbar-width space in a pane with no scrollbar of its own, so cells
  // stay pixel-aligned with a sibling pane that does show one). The row/col
  // header panes have scrolling fully disabled above and never show
  // scrollbars themselves, so there's nothing to compensate for in the
  // common case; if the payoff pane's own scrollbars end up visibly
  // misaligning header cells against payoff cells in some configuration,
  // that's a cosmetic follow-up, not handled here.
}

void TableWidget::UpdateLabelPanels()
{
  UpdateRowGrid();
  UpdateColGrid();
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
  if (m_payoffGrid->IsCellEditControlShown()) {
    m_payoffGrid->DisableCellEditControl();
  }

  if (m_rowGrid->IsCellEditControlShown()) {
    m_rowGrid->DisableCellEditControl();
  }

  if (m_colGrid->IsCellEditControlShown()) {
    m_colGrid->DisableCellEditControl();
  }
}

bool TableWidget::ShowDominance() const { return m_nfgPanel->IsDominanceShown(); }

//=========================================================================
//                      TableWidget: View state
//=========================================================================

bool TableWidget::IsRowPlayerPlacementNoOp(int p_index, int p_player) const
{
  for (int col = 0; col < NumRowPlayers(); ++col) {
    if (GetRowHeaderPlayer(col) == p_player) {
      const int currentIndex = col + 1;
      return p_index == currentIndex || p_index == currentIndex + 1;
    }
  }

  return false;
}

bool TableWidget::IsColPlayerPlacementNoOp(int p_index, int p_player) const
{
  for (int row = 0; row < NumColPlayers(); ++row) {
    if (GetColHeaderPlayer(row) == p_player) {
      const int currentIndex = row + 1;
      return p_index == currentIndex || p_index == currentIndex + 1;
    }
  }

  return false;
}

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
  return new gbtNfgPrintout(this, wxString::FromUTF8(m_doc->GetGame()->GetTitle()));
}

bool TableWidget::GetBitmap(wxBitmap &p_bitmap, int p_marginX, int p_marginY)
{
  const int width =
      (m_rowGrid->CellToRect(0, m_rowGrid->GetNumberCols() - 1).GetRight() +
       m_colGrid->CellToRect(0, m_colGrid->GetNumberCols() - 1).GetRight() + 2 * p_marginX);
  const int height =
      (m_rowGrid->CellToRect(m_rowGrid->GetNumberRows() - 1, 0).GetBottom() +
       m_colGrid->CellToRect(m_colGrid->GetNumberRows() - 1, 0).GetBottom() + 2 * p_marginY);

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
      (m_rowGrid->CellToRect(0, m_rowGrid->GetNumberCols() - 1).GetRight() +
       m_colGrid->CellToRect(0, m_colGrid->GetNumberCols() - 1).GetRight() + 2 * p_marginX);
  const int height =
      (m_rowGrid->CellToRect(m_rowGrid->GetNumberRows() - 1, 0).GetBottom() +
       m_colGrid->CellToRect(m_colGrid->GetNumberRows() - 1, 0).GetBottom() + 2 * p_marginY);

  wxSVGFileDC dc(p_filename, width, height);
  // For some reason, this needs to be initialized
  dc.SetLogicalScale(1.0, 1.0);
  RenderGame(dc, p_marginX, p_marginY);
}

void TableWidget::RenderGame(wxDC &p_dc, int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  const int maxX = (m_rowGrid->CellToRect(0, m_rowGrid->GetNumberCols() - 1).GetRight() +
                    m_colGrid->CellToRect(0, m_colGrid->GetNumberCols() - 1).GetRight());
  const int maxY = (m_rowGrid->CellToRect(m_rowGrid->GetNumberRows() - 1, 0).GetBottom() +
                    m_colGrid->CellToRect(m_colGrid->GetNumberRows() - 1, 0).GetBottom());

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
      m_rowGrid->CellToRect(0, m_rowGrid->GetNumberCols() - 1).GetRight() * scale);
  const int payoffY = static_cast<int>(
      m_colGrid->CellToRect(m_colGrid->GetNumberRows() - 1, 0).GetBottom() * scale);

  p_dc.SetDeviceOrigin(static_cast<int>(posX), payoffY + static_cast<int>(posY));
  DrawGridToDC(m_rowGrid, p_dc);

  p_dc.SetDeviceOrigin(payoffX + static_cast<int>(posX), static_cast<int>(posY));
  DrawGridToDC(m_colGrid, p_dc);

  p_dc.SetDeviceOrigin(payoffX + static_cast<int>(posX), payoffY + static_cast<int>(posY));
  DrawGridToDC(m_payoffGrid, p_dc);
}

void TableWidget::EditStrategies(int player)
{
  const GamePlayer gamePlayer = m_doc->GetGame()->GetPlayer(player);
  EditStrategiesDialog dialog(this, gamePlayer);
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  std::vector<std::string> stableLabels, labels;
  for (int i = 0; i < dialog.NumStrategies(); i++) {
    if (dialog.IsDeleted(i)) {
      continue;
    }
    stableLabels.push_back(dialog.GetStableLabel(i));
    labels.push_back(dialog.GetStrategyLabel(i).ToStdString(wxConvUTF8));
  }

  try {
    m_doc->DoSetStrategies(gamePlayer, stableLabels, labels);
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

void TableWidget::SetPayoffCellValue(int row, int col, const wxString &value)
{
  PureStrategyProfile profile = GetPayoffProfile(row, col);
  GameOutcome outcome = profile->GetOutcome();
  if (!outcome) {
    m_doc->DoNewOutcome(profile);
    profile = GetPayoffProfile(row, col);
    outcome = profile->GetOutcome();
  }

  const int player = GetPayoffPlayerForColumn(col);

  try {
    m_doc->DoSetPayoff(outcome, player, value);
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

bool TableWidget::IsPayoffStrategyDominated(int row, int col, bool strict) const
{
  const PureStrategyProfile profile = GetPayoffProfile(row, col);
  auto player = GetPayoffPlayer(col);
  return GetSupport().IsDominated(profile->GetStrategy(player), strict);
}

GameStrategy TableWidget::GetStrategyByPlayerAndIndex(int player, int strategy) const
{
  auto strategies = GetSupport().GetStrategies(GetSupport().GetGame()->GetPlayer(player));
  return *std::next(strategies.begin(), strategy - 1);
}
} // namespace Gambit::GUI
