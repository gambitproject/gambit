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

#include <map>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif                // WX_PRECOMP
#include <wx/print.h> // for printing support
#include <wx/dcsvg.h> // for SVG output
#include <wx/popupwin.h>

#include <wx/grid.h>

#include "renratio.h" // special renderer/editor for rational numbers
#include "editlabel.h"

#include "gamedoc.h"
#include "nfgpanel.h"
#include "nfgtable.h"
#include "dleditstrategies.h"
#include "dlexcept.h"

namespace Gambit::GUI {

//=========================================================================
//                       class TableGridBase
//=========================================================================

//!
//! This class handles some common overriding of wxGrid behavior
//! common to the grids used in the strategic game display.
//!
class TableGridBase : public wxGrid {
  //!
  //! @name Suppressing the built-in selection highlight & cursor rectangle
  //!
  //@{
  void OnRangeSelecting(wxGridRangeSelectEvent &p_event) { p_event.Veto(); }
  void OnSelectCell(wxGridEvent &p_event) { p_event.Skip(); }
  //@}

protected:
  /// Shows the editor on one click; overridden by the header grids to show
  /// a strategy description popup instead when the grid is read-only.
  virtual void OnCellLeftClick(wxGridEvent &p_event)
  {
    SetGridCursor(p_event.GetRow(), p_event.GetCol());
    EnableCellEditControl();
    p_event.Skip(false);
  }

public:
  explicit TableGridBase(wxWindow *p_parent, wxWindowID p_id = wxID_ANY) : wxGrid(p_parent, p_id)
  {
    SetCellHighlightPenWidth(0);
    SetCellHighlightROPenWidth(0);
    SetRowLabelSize(0);
    SetColLabelSize(0);

    Bind(wxEVT_GRID_RANGE_SELECTING, &TableGridBase::OnRangeSelecting, this);
    Bind(wxEVT_GRID_SELECT_CELL, &TableGridBase::OnSelectCell, this);
    Bind(wxEVT_GRID_CELL_LEFT_CLICK, &TableGridBase::OnCellLeftClick, this);
  }
};

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
//                       class RowHeaderCellRenderer
//=========================================================================

//!
//! Draws the row-player header cells: default string rendering, plus a
//! diagonal dominance-indicator overlay when applicable.
//!
class RowHeaderCellRenderer final : public wxGridCellStringRenderer {
  TableWidget *m_table;

public:
  explicit RowHeaderCellRenderer(TableWidget *p_table) : m_table(p_table) {}

  void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col,
            bool isSelected) override
  {
    wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

    if (!m_table->ShowDominance() || m_table->GetRowHeaderColCount() == 0) {
      return;
    }

    if (m_table->IsRowHeaderStrategyDominated(col, row, false)) {
      const int player = m_table->GetRowHeaderPlayer(col);
      if (m_table->IsRowHeaderStrategyDominated(col, row, true)) {
        dc.SetPen(wxPen(m_table->GetPlayerColor(player), 2, wxPENSTYLE_SOLID));
      }
      else {
        dc.SetPen(wxPen(m_table->GetPlayerColor(player), 1, wxPENSTYLE_SHORT_DASH));
      }
      dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
      dc.DrawLine(rect.x + rect.width, rect.y, rect.x, rect.y + rect.height);
    }
  }

  wxGridCellRenderer *Clone() const override { return new RowHeaderCellRenderer(m_table); }
};

//=========================================================================
//                       class RowPlayerTable
//=========================================================================

class RowPlayerTable final : public wxGridTableBase {
  TableWidget *m_table;
  int m_numRows{0};
  int m_numCols{0};
  RowHeaderCellRenderer *m_renderer;

public:
  explicit RowPlayerTable(TableWidget *p_table)
    : m_table(p_table), m_renderer(new RowHeaderCellRenderer(p_table))
  {
    // Freshly-constructed wxGridCellWorker-derived objects start unreferenced;
    // this is our own table's stake, matched by DecRef() in the destructor.
    // Each SetRenderer() call below additionally consumes one reference per
    // call (that's the ownership contract of that setter), so GetAttr() takes
    // a fresh IncRef() immediately before each one -- the instance itself is
    // reused across all cells and all calls, which is required: wxGrid
    // expects the *same* renderer object identity back on repeated queries
    // for a given cell.
    m_renderer->IncRef();
  }

  ~RowPlayerTable() override { m_renderer->DecRef(); }

  int GetNumberRows() override { return m_numRows; }
  int GetNumberCols() override { return m_numCols; }
  bool IsEmptyCell(int, int) override { return false; }
  wxString GetValue(int row, int col) override;
  void SetValue(int row, int col, const wxString &value) override;
  wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) override;

  /// Reconciles row/col count and span pattern with the current document state.
  void OnUpdate();
};

wxString RowPlayerTable::GetValue(int row, int col)
{
  if (m_table->GetRowHeaderColCount() == 0) {
    return wxT("Payoffs");
  }

  const int player = m_table->GetRowHeaderPlayer(col);
  const int strat = m_table->GetRowHeaderStrategy(col, row);
  return wxString::FromUTF8(m_table->GetStrategyByPlayerAndIndex(player, strat)->GetLabel());
}

void RowPlayerTable::SetValue(int, int, const wxString &)
{
  // Row header cells are read-only: editing a player's strategies goes entirely through
  // TableWidget::EditStrategies(), triggered on left-click rather than through wxGrid's own
  // cell-editing machinery. This override exists only to satisfy the pure virtual
  // wxGridTableBase interface.
}

wxGridCellAttr *RowPlayerTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind)
{
  auto *attr = new wxGridCellAttr();
  attr->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  // wxGridCellStringRenderer's default is to let text that's too wide spill
  // into a neighbouring cell if that cell is otherwise empty -- wrong here,
  // since the "neighbour" past a span boundary is a different strategy's
  // header cell, not empty space.
  attr->SetOverflow(false);
  attr->SetReadOnly(true);

  if (m_table->GetRowHeaderColCount() > 0) {
    attr->SetTextColour(m_table->GetPlayerColor(m_table->GetRowHeaderPlayer(col)));
  }
  else {
    attr->SetTextColour(*wxBLACK);
  }

  // GetAttr() constructs a fresh attr on every call, so the span has to be
  // computed and set directly here rather than via wxGrid::SetCellSize()
  // -- that call (and GetCellSize()) themselves go through GetAttr(), which
  // would recurse infinitely if this override tried to consult it. Only
  // the anchor (first row of each span group) gets a non-trivial size;
  // interior rows are left at the default 1x1. Computed unconditionally
  // (matching ColPlayerTable::GetAttr) rather than gated on
  // GetRowHeaderColCount() > 0, for the same reason: don't rely on a
  // formula's edge-case behaviour matching whichever branch happens to
  // call it.
  const int span = m_table->GetRowHeaderRowSpan(col);
  if (span > 1 && row % span == 0) {
    attr->SetSize(span, 1);
  }

  attr->SetBackgroundColour(*wxLIGHT_GREY);
  m_renderer->IncRef();
  attr->SetRenderer(m_renderer);

  return attr;
}

void RowPlayerTable::OnUpdate()
{
  wxGrid *view = GetView();

  const int newRows = m_table->GetRowHeaderRowCount();
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

  const int newCols = std::max(m_table->GetRowHeaderColCount(), 1);
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

  // Spans are computed directly in GetAttr() (see the comment there), so
  // there's no separate SetCellSize() pass needed here -- just repaint.
  if (view) {
    view->Refresh();
  }
}

//=========================================================================
//                       class RowPlayerGrid
//=========================================================================

namespace {
wxString GetStrategyDescription(const GameStrategy &p_strategy)
{
  if (!p_strategy->GetGame()->IsTree()) {
    return {};
  }

  wxString description;
  for (const auto &infoset : p_strategy->GetPlayer()->GetInfosets()) {
    const auto action = p_strategy->GetAction(infoset);
    if (!action) {
      continue;
    }

    if (!description.empty()) {
      description << wxT("\n");
    }
    description << wxString::Format(_("Information set %d"), infoset->GetNumber());
    if (!infoset->GetLabel().empty()) {
      description << wxT(" (") << wxString(infoset->GetLabel().c_str(), *wxConvCurrent)
                  << wxT(")");
    }
    description << wxT(": ");
    if (action->GetLabel().empty()) {
      description << wxString::Format(_("action %d"), action->GetNumber());
    }
    else {
      description << wxString(action->GetLabel().c_str(), *wxConvCurrent);
    }
  }
  return description;
}

// The strategy's index in the flattened, game-wide numbering used by
// AnalysisOutput::GetStrategyProb/GetStrategyValue (players in Game::GetPlayers() order,
// each contributing their own strategies in order) -- distinct from
// GameStrategy::GetNumber(), which is only the index within its own player.
int GetGlobalStrategyIndex(const GameStrategy &p_strategy)
{
  int index = 0;
  for (const auto &player : p_strategy->GetGame()->GetPlayers()) {
    if (player == p_strategy->GetPlayer()) {
      break;
    }
    index += static_cast<int>(player->GetStrategies().size());
  }
  return index + p_strategy->GetNumber();
}

bool HasStrategyInfo(GameDocument *p_doc, const GameStrategy &p_strategy)
{
  return !GetStrategyDescription(p_strategy).empty() ||
         p_doc->GetWorkspace().GetCurrentProfile() > 0;
}

// A hover tooltip for a row/col header's strategy cell: the strategy's tree-derived
// description (if the nfg is derived from a tree), and -- when a profile is selected --
// its probability and expected payoff under the current profile (the classic "value of
// this pure strategy against the others' current play" readout used to check best
// responses), plus its player's own overall current-profile payoff for context. Built
// once and re-populated on each hover, mirroring NodeInfoPopup in efgtooltip.cc.
class StrategyInfoPopup final : public wxPopupTransientWindow {
public:
  explicit StrategyInfoPopup(wxWindow *p_parent) : wxPopupTransientWindow(p_parent, wxBORDER_NONE)
  {
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
    BuildControls();
  }

  void ShowForStrategy(GameDocument *p_doc, const GameStrategy &p_strategy,
                       const wxPoint &p_anchor)
  {
    const wxString label = wxString::FromUTF8(p_strategy->GetLabel());
    m_heading->SetLabel(wxString::Format(_("Strategy %s"), label));

    const wxString description = GetStrategyDescription(p_strategy);
    m_description->SetLabel(description);
    m_contentPanel->GetSizer()->Show(m_description, !description.empty(), true);

    const bool hasProfile = p_doc->GetWorkspace().GetCurrentProfile() > 0;
    if (hasProfile) {
      const AnalysisOutput &profiles = p_doc->GetWorkspace().GetProfiles();
      const int index = GetGlobalStrategyIndex(p_strategy);
      const GamePlayer player = p_strategy->GetPlayer();
      const wxColour color = p_doc->GetStyle().GetPlayerColor(player);

      wxString playerLabel = wxString::FromUTF8(player->GetLabel());
      if (playerLabel.empty()) {
        playerLabel = wxString::Format(_("Player %d"), player->GetNumber());
      }

      m_probText->SetForegroundColour(color);
      m_probText->SetLabel(_("Pr(played): ") +
                           wxString::FromUTF8(profiles.GetStrategyProb(index)));
      m_valueText->SetForegroundColour(color);
      m_valueText->SetLabel(_("Payoff if played: ") +
                            wxString::FromUTF8(profiles.GetStrategyValue(index)));
      m_payoffText->SetForegroundColour(color);
      m_payoffText->SetLabel(playerLabel + _(" payoff: ") +
                             wxString::FromUTF8(profiles.GetPayoff(player->GetNumber())));
    }
    m_contentPanel->GetSizer()->Show(m_profileSizer, hasProfile, true);

    m_contentPanel->GetSizer()->Layout();
    Fit();
    Position(p_anchor, wxSize(FromDIP(8), FromDIP(8)));
    Popup();
  }

private:
  void BuildControls()
  {
    auto *popupSizer = new wxBoxSizer(wxVERTICAL);

    m_contentPanel = new wxPanel(this);
    m_contentPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    auto *outerSizer = new wxBoxSizer(wxVERTICAL);

    m_heading = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
    wxFont headingFont = m_heading->GetFont();
    headingFont.SetWeight(wxFONTWEIGHT_BOLD);
    headingFont.SetPointSize(headingFont.GetPointSize() + 1);
    m_heading->SetFont(headingFont);
    outerSizer->Add(m_heading, 0, wxLEFT | wxRIGHT | wxTOP, FromDIP(12));

    m_description = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
    m_description->Wrap(FromDIP(420));
    outerSizer->Add(m_description, 0, wxALL, FromDIP(12));

    m_profileSizer = new wxBoxSizer(wxVERTICAL);
    m_probText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
    m_profileSizer->Add(m_probText, 0, wxTOP, FromDIP(2));
    m_valueText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
    m_profileSizer->Add(m_valueText, 0, wxTOP, FromDIP(2));
    m_payoffText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
    m_profileSizer->Add(m_payoffText, 0, wxTOP, FromDIP(2));
    outerSizer->Add(m_profileSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(12));

    m_contentPanel->SetSizer(outerSizer);
    popupSizer->Add(m_contentPanel, 1, wxEXPAND | wxALL, FromDIP(1));
    SetSizerAndFit(popupSizer);
  }

  wxPanel *m_contentPanel;
  wxStaticText *m_heading;
  wxStaticText *m_description;
  wxBoxSizer *m_profileSizer;
  wxStaticText *m_probText;
  wxStaticText *m_valueText;
  wxStaticText *m_payoffText;
};

} // namespace

class RowPlayerGrid final : public TableGridBase {
  TableWidget *m_table;
  RowPlayerTable *m_gridTable;
  StrategyInfoPopup *m_infoPopup;
  wxTimer m_hoverTimer;
  int m_hoverRow{-1}, m_hoverCol{-1};

  void OnCellLeftClick(wxGridEvent &) override;
  void OnCellRightClick(wxGridEvent &);
  void OnMotion(wxMouseEvent &);
  void OnLeaveWindow(wxMouseEvent &);
  void OnHoverTimer(wxTimerEvent &);
  void DismissInfo();

  void ShowPlacementMenu(int p_col, const wxPoint &p_pos);

public:
  explicit RowPlayerGrid(TableWidget *p_parent);

  void OnUpdate() { m_gridTable->OnUpdate(); }
};

RowPlayerGrid::RowPlayerGrid(TableWidget *p_parent)
  : TableGridBase(p_parent, wxID_ANY), m_table(p_parent),
    m_gridTable(new RowPlayerTable(p_parent)), m_infoPopup(new StrategyInfoPopup(this))
{
  SetTable(m_gridTable, true);
  SetGridLineColour(*wxBLACK);
  EnableScrolling(false, false);
  ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
  // EnableScrolling(false, false) doesn't stop wxGrid's own mouse-wheel
  // handling, which bypasses the base wxScrolledWindow gate -- this pane
  // must only ever move in response to the payoff grid's scroll sync.
  Bind(wxEVT_MOUSEWHEEL, [](wxMouseEvent &) {});

  Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &RowPlayerGrid::OnCellRightClick, this);
  // wxGrid is a composite widget -- plain (non-command) mouse events like these are
  // delivered to the internal grid window, not this outer wxGrid object, so they have to
  // be bound there instead. wxEVT_GRID_CELL_*_CLICK don't have this problem: those are
  // wxGridEvents, which wxGrid itself generates and dispatches to the outer object.
  GetGridWindow()->Bind(wxEVT_MOTION, &RowPlayerGrid::OnMotion, this);
  GetGridWindow()->Bind(wxEVT_LEAVE_WINDOW, &RowPlayerGrid::OnLeaveWindow, this);
  m_hoverTimer.SetOwner(this);
  Bind(wxEVT_TIMER, &RowPlayerGrid::OnHoverTimer, this);
}

void RowPlayerGrid::OnCellLeftClick(wxGridEvent &p_event)
{
  DismissInfo();

  if (m_table->GetRowHeaderColCount() == 0) {
    TableGridBase::OnCellLeftClick(p_event);
    return;
  }

  if (m_table->IsReadOnly()) {
    return;
  }

  m_table->EditStrategies(m_table->GetRowHeaderPlayer(p_event.GetCol()));
}

void RowPlayerGrid::OnCellRightClick(wxGridEvent &p_event)
{
  DismissInfo();
  ShowPlacementMenu(p_event.GetCol(), p_event.GetPosition());
}

void RowPlayerGrid::OnMotion(wxMouseEvent &p_event)
{
  if (m_table->GetRowHeaderColCount() > 0) {
    const wxGridCellCoords coords = XYToCell(p_event.GetPosition());
    if (coords.GetRow() != m_hoverRow || coords.GetCol() != m_hoverCol) {
      DismissInfo();
      m_hoverRow = coords.GetRow();
      m_hoverCol = coords.GetCol();
      if (m_hoverRow >= 0 && m_hoverCol >= 0) {
        m_hoverTimer.StartOnce(400);
      }
    }
  }
  p_event.Skip();
}

void RowPlayerGrid::OnLeaveWindow(wxMouseEvent &p_event)
{
  DismissInfo();
  p_event.Skip();
}

void RowPlayerGrid::DismissInfo()
{
  m_hoverTimer.Stop();
  m_hoverRow = m_hoverCol = -1;
  if (m_infoPopup->IsShown()) {
    m_infoPopup->Dismiss();
  }
}

void RowPlayerGrid::OnHoverTimer(wxTimerEvent &)
{
  if (m_hoverRow < 0 || m_hoverCol < 0) {
    return;
  }
  const int player = m_table->GetRowHeaderPlayer(m_hoverCol);
  const int strategy = m_table->GetRowHeaderStrategy(m_hoverCol, m_hoverRow);
  const auto gameStrategy = m_table->GetStrategyByPlayerAndIndex(player, strategy);
  GameDocument *doc = m_table->GetDocument();
  if (!HasStrategyInfo(doc, gameStrategy)) {
    return;
  }
  const wxRect cellRect = CellToRect(m_hoverRow, m_hoverCol);
  const wxPoint anchor = GetGridWindow()->ClientToScreen(cellRect.GetBottomLeft());
  m_infoPopup->ShowForStrategy(doc, gameStrategy, anchor);
}

// Right-clicking a row-header cell offers to place any player at this position (before
// or after the player currently occupying it), replacing what used to be a drag of a
// player icon from the (now-removed) left-hand player panel -- that never worked very
// well, and didn't survive the panel's removal anyway. When there are no row players yet,
// the only choice is which player to use as the (single) row player.
void RowPlayerGrid::ShowPlacementMenu(int p_col, const wxPoint &p_pos)
{
  const bool hasRowPlayers = m_table->GetRowHeaderColCount() > 0;
  const int beforeIndex = hasRowPlayers ? p_col + 1 : 1;
  const int afterIndex = beforeIndex + 1;

  wxMenu menu;
  wxMenu *beforeMenu = hasRowPlayers ? new wxMenu : nullptr;
  wxMenu *afterMenu = hasRowPlayers ? new wxMenu : nullptr;
  wxMenu *useMenu = hasRowPlayers ? nullptr : new wxMenu;
  std::map<int, std::pair<int, int>> placements; // menu id -> (index, player)

  for (const auto &player : m_table->GetDocument()->GetGame()->GetPlayers()) {
    wxString label = wxString::FromUTF8(player->GetLabel());
    if (label.empty()) {
      label = wxString::Format(_("Player %d"), player->GetNumber());
    }
    const wxBitmap swatch = MakeColorSwatch(m_table->GetPlayerColor(player->GetNumber()));

    if (!hasRowPlayers) {
      const int id = wxWindow::NewControlId();
      auto *item = new wxMenuItem(useMenu, id, label);
      item->SetBitmap(swatch);
      useMenu->Append(item);
      placements[id] = {1, player->GetNumber()};
      continue;
    }

    if (!m_table->IsRowPlayerPlacementNoOp(beforeIndex, player->GetNumber())) {
      const int id = wxWindow::NewControlId();
      auto *item = new wxMenuItem(beforeMenu, id, label);
      item->SetBitmap(swatch);
      beforeMenu->Append(item);
      placements[id] = {beforeIndex, player->GetNumber()};
    }
    if (!m_table->IsRowPlayerPlacementNoOp(afterIndex, player->GetNumber())) {
      const int id = wxWindow::NewControlId();
      auto *item = new wxMenuItem(afterMenu, id, label);
      item->SetBitmap(swatch);
      afterMenu->Append(item);
      placements[id] = {afterIndex, player->GetNumber()};
    }
  }

  if (hasRowPlayers) {
    if (beforeMenu->GetMenuItemCount() > 0) {
      menu.AppendSubMenu(beforeMenu, _("Place player before"));
    }
    else {
      delete beforeMenu;
    }
    if (afterMenu->GetMenuItemCount() > 0) {
      menu.AppendSubMenu(afterMenu, _("Place player after"));
    }
    else {
      delete afterMenu;
    }
  }
  else {
    menu.AppendSubMenu(useMenu, _("Use as row player"));
  }

  if (menu.GetMenuItemCount() == 0) {
    return;
  }

  const int selection = GetPopupMenuSelectionFromUser(menu, p_pos);
  const auto it = placements.find(selection);
  if (it == placements.end()) {
    return;
  }

  try {
    m_table->SetRowPlayer(it->second.first, it->second.second);
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

//=========================================================================
//                       class ColHeaderCellRenderer
//=========================================================================

//!
//! Draws the column-player header cells: default string rendering, plus a
//! diagonal dominance-indicator overlay when applicable.
//!
class ColHeaderCellRenderer final : public wxGridCellStringRenderer {
  TableWidget *m_table;

public:
  explicit ColHeaderCellRenderer(TableWidget *p_table) : m_table(p_table) {}

  void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col,
            bool isSelected) override
  {
    wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

    if (!m_table->ShowDominance() || m_table->GetColHeaderRowCount() == 0) {
      return;
    }

    if (m_table->IsColHeaderStrategyDominated(row, col, false)) {
      const int player = m_table->GetColHeaderPlayer(row);
      if (m_table->IsColHeaderStrategyDominated(row, col, true)) {
        dc.SetPen(wxPen(m_table->GetPlayerColor(player), 2, wxPENSTYLE_SOLID));
      }
      else {
        dc.SetPen(wxPen(m_table->GetPlayerColor(player), 1, wxPENSTYLE_SHORT_DASH));
      }
      dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
      dc.DrawLine(rect.x + rect.width, rect.y, rect.x, rect.y + rect.height);
    }
  }

  wxGridCellRenderer *Clone() const override { return new ColHeaderCellRenderer(m_table); }
};

//=========================================================================
//                       class ColPlayerTable
//=========================================================================

class ColPlayerTable final : public wxGridTableBase {
  TableWidget *m_table;
  int m_numRows{0};
  int m_numCols{0};
  ColHeaderCellRenderer *m_renderer;

public:
  explicit ColPlayerTable(TableWidget *p_table)
    : m_table(p_table), m_renderer(new ColHeaderCellRenderer(p_table))
  {
    m_renderer->IncRef();
  }

  ~ColPlayerTable() override { m_renderer->DecRef(); }

  int GetNumberRows() override { return m_numRows; }
  int GetNumberCols() override { return m_numCols; }
  bool IsEmptyCell(int, int) override { return false; }
  wxString GetValue(int row, int col) override;
  void SetValue(int row, int col, const wxString &value) override;
  wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) override;

  void OnUpdate();
};

wxString ColPlayerTable::GetValue(int row, int col)
{
  if (m_table->GetColHeaderRowCount() == 0) {
    return wxT("Payoffs");
  }

  const int player = m_table->GetColHeaderPlayer(row);
  const int strat = m_table->GetColHeaderStrategy(row, col);
  return wxString::FromUTF8(m_table->GetStrategyByPlayerAndIndex(player, strat)->GetLabel());
}

void ColPlayerTable::SetValue(int, int, const wxString &)
{
  // Column header cells are read-only: editing a player's strategies goes entirely through
  // TableWidget::EditStrategies(), triggered on left-click rather than through wxGrid's own
  // cell-editing machinery. This override exists only to satisfy the pure virtual
  // wxGridTableBase interface.
}

wxGridCellAttr *ColPlayerTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind)
{
  auto *attr = new wxGridCellAttr();
  attr->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  // wxGridCellStringRenderer's default is to let text that's too wide spill
  // into a neighbouring cell if that cell is otherwise empty -- wrong here,
  // since the "neighbour" past a span boundary is a different strategy's
  // header cell, not empty space.
  attr->SetOverflow(false);
  attr->SetReadOnly(true);

  if (m_table->GetColHeaderRowCount() > 0) {
    attr->SetTextColour(m_table->GetPlayerColor(m_table->GetColHeaderPlayer(row)));
  }
  else {
    attr->SetTextColour(*wxBLACK);
  }

  // See the equivalent comment in RowPlayerTable::GetAttr: span has to be
  // computed directly here, not via SetCellSize()/GetCellSize(), which
  // would recurse back into this same override. Computed unconditionally
  // (not just when there are real column players): with zero column
  // players, GetColHeaderColSpan's formula naturally evaluates to the full
  // width, merging the "Payoffs" placeholder header across every column.
  const int span = m_table->GetColHeaderColSpan(row);
  if (span > 1 && col % span == 0) {
    attr->SetSize(1, span);
  }

  attr->SetBackgroundColour(*wxLIGHT_GREY);
  m_renderer->IncRef();
  attr->SetRenderer(m_renderer);

  return attr;
}

void ColPlayerTable::OnUpdate()
{
  wxGrid *view = GetView();

  const int newCols = m_table->GetColHeaderColCount();
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

  const int newRows = std::max(m_table->GetColHeaderRowCount(), 1);
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

  // Spans are computed directly in GetAttr() (see the comment there), so
  // there's no separate SetCellSize() pass needed here -- just repaint.
  if (view) {
    view->Refresh();
  }
}

//=========================================================================
//                       class ColPlayerGrid
//=========================================================================

class ColPlayerGrid final : public TableGridBase {
  TableWidget *m_table;
  ColPlayerTable *m_gridTable;
  StrategyInfoPopup *m_infoPopup;
  wxTimer m_hoverTimer;
  int m_hoverRow{-1}, m_hoverCol{-1};

  void OnCellLeftClick(wxGridEvent &) override;
  void OnCellRightClick(wxGridEvent &);
  void OnMotion(wxMouseEvent &);
  void OnLeaveWindow(wxMouseEvent &);
  void OnHoverTimer(wxTimerEvent &);
  void DismissInfo();

  void ShowPlacementMenu(int p_row, const wxPoint &p_pos);

public:
  explicit ColPlayerGrid(TableWidget *p_parent);

  void OnUpdate() { m_gridTable->OnUpdate(); }
};

ColPlayerGrid::ColPlayerGrid(TableWidget *p_parent)
  : TableGridBase(p_parent, wxID_ANY), m_table(p_parent),
    m_gridTable(new ColPlayerTable(p_parent)), m_infoPopup(new StrategyInfoPopup(this))
{
  SetTable(m_gridTable, true);
  SetGridLineColour(*wxBLACK);
  wxWindow::SetBackgroundColour(*wxLIGHT_GREY);
  EnableScrolling(false, false);
  ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
  // See the equivalent comment in RowPlayerGrid's constructor.
  Bind(wxEVT_MOUSEWHEEL, [](wxMouseEvent &) {});

  Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &ColPlayerGrid::OnCellRightClick, this);
  // See the equivalent comment in RowPlayerGrid's constructor.
  GetGridWindow()->Bind(wxEVT_MOTION, &ColPlayerGrid::OnMotion, this);
  GetGridWindow()->Bind(wxEVT_LEAVE_WINDOW, &ColPlayerGrid::OnLeaveWindow, this);
  m_hoverTimer.SetOwner(this);
  Bind(wxEVT_TIMER, &ColPlayerGrid::OnHoverTimer, this);
}

void ColPlayerGrid::OnCellLeftClick(wxGridEvent &p_event)
{
  DismissInfo();

  if (m_table->GetColHeaderRowCount() == 0) {
    TableGridBase::OnCellLeftClick(p_event);
    return;
  }

  if (m_table->IsReadOnly()) {
    return;
  }

  m_table->EditStrategies(m_table->GetColHeaderPlayer(p_event.GetRow()));
}

void ColPlayerGrid::OnCellRightClick(wxGridEvent &p_event)
{
  DismissInfo();
  ShowPlacementMenu(p_event.GetRow(), p_event.GetPosition());
}

void ColPlayerGrid::OnMotion(wxMouseEvent &p_event)
{
  if (m_table->GetColHeaderRowCount() > 0) {
    const wxGridCellCoords coords = XYToCell(p_event.GetPosition());
    if (coords.GetRow() != m_hoverRow || coords.GetCol() != m_hoverCol) {
      DismissInfo();
      m_hoverRow = coords.GetRow();
      m_hoverCol = coords.GetCol();
      if (m_hoverRow >= 0 && m_hoverCol >= 0) {
        m_hoverTimer.StartOnce(400);
      }
    }
  }
  p_event.Skip();
}

void ColPlayerGrid::OnLeaveWindow(wxMouseEvent &p_event)
{
  DismissInfo();
  p_event.Skip();
}

void ColPlayerGrid::DismissInfo()
{
  m_hoverTimer.Stop();
  m_hoverRow = m_hoverCol = -1;
  if (m_infoPopup->IsShown()) {
    m_infoPopup->Dismiss();
  }
}

void ColPlayerGrid::OnHoverTimer(wxTimerEvent &)
{
  if (m_hoverRow < 0 || m_hoverCol < 0) {
    return;
  }
  const int player = m_table->GetColHeaderPlayer(m_hoverRow);
  const int strategy = m_table->GetColHeaderStrategy(m_hoverRow, m_hoverCol);
  const auto gameStrategy = m_table->GetStrategyByPlayerAndIndex(player, strategy);
  GameDocument *doc = m_table->GetDocument();
  if (!HasStrategyInfo(doc, gameStrategy)) {
    return;
  }
  const wxRect cellRect = CellToRect(m_hoverRow, m_hoverCol);
  const wxPoint anchor = GetGridWindow()->ClientToScreen(cellRect.GetBottomLeft());
  m_infoPopup->ShowForStrategy(doc, gameStrategy, anchor);
}

// See the equivalent comment on RowPlayerGrid::ShowPlacementMenu.
void ColPlayerGrid::ShowPlacementMenu(int p_row, const wxPoint &p_pos)
{
  const bool hasColPlayers = m_table->GetColHeaderRowCount() > 0;
  const int beforeIndex = hasColPlayers ? p_row + 1 : 1;
  const int afterIndex = beforeIndex + 1;

  wxMenu menu;
  wxMenu *beforeMenu = hasColPlayers ? new wxMenu : nullptr;
  wxMenu *afterMenu = hasColPlayers ? new wxMenu : nullptr;
  wxMenu *useMenu = hasColPlayers ? nullptr : new wxMenu;
  std::map<int, std::pair<int, int>> placements; // menu id -> (index, player)

  for (const auto &player : m_table->GetDocument()->GetGame()->GetPlayers()) {
    wxString label = wxString::FromUTF8(player->GetLabel());
    if (label.empty()) {
      label = wxString::Format(_("Player %d"), player->GetNumber());
    }
    const wxBitmap swatch = MakeColorSwatch(m_table->GetPlayerColor(player->GetNumber()));

    if (!hasColPlayers) {
      const int id = wxWindow::NewControlId();
      auto *item = new wxMenuItem(useMenu, id, label);
      item->SetBitmap(swatch);
      useMenu->Append(item);
      placements[id] = {1, player->GetNumber()};
      continue;
    }

    if (!m_table->IsColPlayerPlacementNoOp(beforeIndex, player->GetNumber())) {
      const int id = wxWindow::NewControlId();
      auto *item = new wxMenuItem(beforeMenu, id, label);
      item->SetBitmap(swatch);
      beforeMenu->Append(item);
      placements[id] = {beforeIndex, player->GetNumber()};
    }
    if (!m_table->IsColPlayerPlacementNoOp(afterIndex, player->GetNumber())) {
      const int id = wxWindow::NewControlId();
      auto *item = new wxMenuItem(afterMenu, id, label);
      item->SetBitmap(swatch);
      afterMenu->Append(item);
      placements[id] = {afterIndex, player->GetNumber()};
    }
  }

  if (hasColPlayers) {
    if (beforeMenu->GetMenuItemCount() > 0) {
      menu.AppendSubMenu(beforeMenu, _("Place player before"));
    }
    else {
      delete beforeMenu;
    }
    if (afterMenu->GetMenuItemCount() > 0) {
      menu.AppendSubMenu(afterMenu, _("Place player after"));
    }
    else {
      delete afterMenu;
    }
  }
  else {
    menu.AppendSubMenu(useMenu, _("Use as column player"));
  }

  if (menu.GetMenuItemCount() == 0) {
    return;
  }

  const int selection = GetPopupMenuSelectionFromUser(menu, p_pos);
  const auto it = placements.find(selection);
  if (it == placements.end()) {
    return;
  }

  try {
    m_table->SetColPlayer(it->second.first, it->second.second);
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

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
  // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)
  m_payoffGrid = new PayoffGrid(this);
  m_rowGrid = new RowPlayerGrid(this);
  m_colGrid = new ColPlayerGrid(this);
  // NOLINTEND(cppcoreguidelines-prefer-member-initializer)
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

void TableWidget::UpdatePayoffPanel() { dynamic_cast<PayoffGrid *>(m_payoffGrid)->OnUpdate(); }

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
  dynamic_cast<RowPlayerGrid *>(m_rowGrid)->OnUpdate();
  dynamic_cast<ColPlayerGrid *>(m_colGrid)->OnUpdate();
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
