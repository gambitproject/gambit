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

#include <algorithm>
#include <set>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include <wx/grid.h>

#include "renratio.h" // special renderer/editor for rational numbers

#include "dlexcept.h"
#include "editlabel.h"
#include "gamedoc.h"
#include "nfgtable.h"

namespace Gambit::GUI {

/// Background for every payoff cell sharing the outcome currently under the mouse.
static const wxColour kHoverOutcomeBg(220, 240, 234);
/// Background for the cells currently selected for a "Set outcome" action -- distinct
/// from the hover highlight above, and from every player's payoff colour.
static const wxColour kSelectionBg(224, 224, 224);

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
  attr->SetBackgroundColour(m_table->IsPayoffCellHighlighted(row, col) ? kHoverOutcomeBg
                                                                       : *wxWHITE);
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
//                       class SetOutcomeDialog
//=========================================================================

namespace {
const wxColour kInvalidFieldBg(255, 220, 220);
} // namespace

//!
//! Prompts for a label and one payoff per player, to create the outcome attached to a
//! set of contingencies -- whether that's merging several into one (a multi-contingency
//! selection), splitting one off from a shared outcome (a single contingency out of a
//! larger group), or just editing a shared outcome's own label/payoffs in place (a
//! selection that already exactly matches one outcome's membership).
//!
class SetOutcomeDialog final : public wxDialog {
  struct Row {
    GamePlayer player;
    wxTextCtrl *payoffCtrl;
  };

  const Game m_game;
  const std::set<const GameOutcomeRep *> m_ignoreForLabel;
  LabelTextCtrl *m_labelCtrl;
  std::vector<Row> m_payoffRows;
  wxStaticText *m_errorText;
  wxButton *m_okButton;
  wxColour m_defaultBg;

  //!
  //! Re-validates every field, recolouring each to flag whether it is currently valid,
  //! and returns a description of the first problem found (or an empty string if none).
  //!
  wxString ValidateFields()
  {
    const wxString label = m_labelCtrl->GetNormalizedValue();
    const bool labelEmpty = label.empty();
    bool labelDuplicate = false;
    if (!labelEmpty) {
      for (const auto &outcome : m_game->GetOutcomes()) {
        if (!m_ignoreForLabel.contains(outcome.get()) &&
            wxString::FromUTF8(outcome->GetLabel()) == label) {
          labelDuplicate = true;
          break;
        }
      }
    }
    m_labelCtrl->SetBackgroundColour((labelEmpty || labelDuplicate) ? kInvalidFieldBg
                                                                    : m_defaultBg);
    m_labelCtrl->Refresh();

    wxString message;
    if (labelEmpty) {
      message = _("Outcome label cannot be empty.");
    }
    else if (labelDuplicate) {
      message = _("Outcome label must be unique.");
    }

    for (const auto &row : m_payoffRows) {
      bool invalid = false;
      try {
        lexical_cast<Rational>(row.payoffCtrl->GetValue().ToStdString());
      }
      catch (const std::exception &) {
        invalid = true;
      }
      row.payoffCtrl->SetBackgroundColour(invalid ? kInvalidFieldBg : m_defaultBg);
      row.payoffCtrl->Refresh();
      if (invalid && message.empty()) {
        message = wxString::Format(_("Payoff for %s is not a valid number."),
                                   wxString::FromUTF8(row.player->GetLabel()));
      }
    }
    return message;
  }

  void UpdateValidation()
  {
    const wxString message = ValidateFields();
    m_errorText->SetLabel(message);
    m_errorText->Show(!message.empty());
    m_okButton->Enable(message.empty());
    Layout();
    Fit();
  }

  void OnOK(wxCommandEvent &p_event)
  {
    if (!ValidateFields().empty()) {
      wxBell();
      UpdateValidation();
      return;
    }
    p_event.Skip();
  }

public:
  SetOutcomeDialog(wxWindow *p_parent, const Game &p_game, const wxString &p_title,
                   const wxString &p_label, const std::vector<wxString> &p_payoffValues,
                   const std::set<const GameOutcomeRep *> &p_ignoreForLabel)
    : wxDialog(p_parent, wxID_ANY, p_title), m_game(p_game), m_ignoreForLabel(p_ignoreForLabel)
  {
    auto *topSizer = new wxBoxSizer(wxVERTICAL);

    auto *labelSizer = new wxBoxSizer(wxHORIZONTAL);
    labelSizer->Add(new wxStaticText(this, wxID_ANY, _("Label")), 0,
                    wxALL | wxALIGN_CENTRE_VERTICAL, 5);
    m_labelCtrl = new LabelTextCtrl(this, wxID_ANY, p_label);
    labelSizer->Add(m_labelCtrl, 1, wxALL | wxEXPAND, 5);
    topSizer->Add(labelSizer, 0, wxEXPAND | wxALL, 5);

    auto *payoffSizer = new wxFlexGridSizer(2, FromDIP(5), FromDIP(10));
    payoffSizer->AddGrowableCol(1, 1);
    const auto players = p_game->GetPlayers();
    size_t index = 0;
    for (const auto &player : players) {
      wxString playerLabel = wxString::FromUTF8(player->GetLabel());
      if (playerLabel.empty()) {
        playerLabel = wxString::Format(_("Player %d"), player->GetNumber());
      }
      payoffSizer->Add(new wxStaticText(this, wxID_ANY, playerLabel), 0, wxALIGN_CENTRE_VERTICAL);
      auto *ctrl = new wxTextCtrl(this, wxID_ANY, p_payoffValues.at(index));
      ctrl->Bind(wxEVT_TEXT, [this](wxCommandEvent &p_textEvent) {
        UpdateValidation();
        p_textEvent.Skip();
      });
      payoffSizer->Add(ctrl, 1, wxEXPAND);
      m_payoffRows.push_back({player, ctrl});
      ++index;
    }
    topSizer->Add(payoffSizer, 0, wxEXPAND | wxALL, 5);

    m_defaultBg = m_labelCtrl->GetBackgroundColour();

    m_errorText = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_errorText->SetForegroundColour(*wxRED);
    m_errorText->Wrap(FromDIP(260));
    m_errorText->Hide();
    topSizer->Add(m_errorText, 0, wxEXPAND | wxALL, 5);

    auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
    topSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);

    SetSizer(topSizer);
    Fit();
    CentreOnParent();

    m_labelCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent &p_textEvent) {
      UpdateValidation();
      p_textEvent.Skip();
    });
    m_okButton = static_cast<wxButton *>(FindWindow(wxID_OK));
    Bind(wxEVT_BUTTON, &SetOutcomeDialog::OnOK, this, wxID_OK);

    UpdateValidation();
  }

  wxString GetOutcomeLabel() const { return m_labelCtrl->GetNormalizedValue(); }

  std::vector<Number> GetPayoffs() const
  {
    std::vector<Number> payoffs;
    payoffs.reserve(m_payoffRows.size());
    for (const auto &row : m_payoffRows) {
      payoffs.emplace_back(row.payoffCtrl->GetValue().ToStdString());
    }
    return payoffs;
  }
};

//=========================================================================
//                       class PayoffGrid
//=========================================================================

class PayoffGrid final : public TableGridBase {
  TableWidget *m_table;
  PayoffTable *m_gridTable;
  // Guards OnRangeSelecting's own SelectBlock() call against re-entering itself, since
  // that call fires another RANGE_SELECTING event in turn.
  bool m_snappingSelection = false;

  /// @name Overriding wxGrid behavior for event-handling
  //@{
  /// Implement custom tab-traversal behavior
  void OnKeyDown(wxKeyEvent &);
  void OnCharHook(wxKeyEvent &);
  void HandleTabTraversal(wxKeyEvent &);
  void MoveEditorByTab(bool p_backwards);
  /// Highlight every cell sharing the outcome under the mouse
  void OnMotion(wxMouseEvent &);
  void OnLeaveWindow(wxMouseEvent &);
  /// Allows range selection (unlike the header grids), snapped live as the drag/shift-click
  /// proceeds to cover every payoff column of each contingency it touches -- so the visible
  /// selection always matches what a "Set outcome" action on it would cover, selecting
  /// being otherwise cell-grained, with no notion of a contingency.
  void OnRangeSelecting(wxGridRangeSelectEvent &) override;
  /// The base class's own click handling drives the cursor and cell editor, and swallows
  /// the event outright (never reaching wxGrid's own default handling, which is what
  /// would otherwise clear a range selection on a plain click) -- so a plain click here
  /// must clear it itself, or nothing ever would.
  void OnCellLeftClick(wxGridEvent &) override;
  /// Offer to merge the selected contingencies into one outcome, split one off from a
  /// shared outcome, or clear their outcomes entirely
  void OnCellRightClick(wxGridEvent &);
  //@}

  /// The distinct contingencies covered by the current selection, deduplicated across
  /// the several payoff columns each contingency spans -- or just the given cell, if
  /// nothing is currently selected (e.g. a right-click with no preceding drag/shift-click).
  std::vector<PureStrategyProfile> GetSelectedContingencies(int p_fallbackRow,
                                                            int p_fallbackCol) const;
  /// The one outcome shared by every given contingency, if they all have the same
  /// non-null outcome -- the null handle otherwise (mixed outcomes, or none at all).
  static GameOutcome GetSharedOutcome(const std::vector<PureStrategyProfile> &p_profiles);
  void ShowSetOutcomeDialog(const std::vector<PureStrategyProfile> &p_profiles);
  void ShowEditOutcomeDialog(const GameOutcome &p_outcome);

public:
  explicit PayoffGrid(TableWidget *p_parent);

  void OnUpdate() { m_gridTable->OnUpdate(); }
};

PayoffGrid::PayoffGrid(TableWidget *p_parent)
  : TableGridBase(p_parent, wxID_ANY), m_table(p_parent), m_gridTable(new PayoffTable(p_parent))
{
  SetTable(m_gridTable, true);
  SetSelectionBackground(kSelectionBg);
  SetSelectionForeground(*wxBLACK);

  Bind(wxEVT_KEY_DOWN, &PayoffGrid::OnKeyDown, this);
  Bind(wxEVT_CHAR_HOOK, &PayoffGrid::OnCharHook, this);
  Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &PayoffGrid::OnCellRightClick, this);
  GetGridWindow()->Bind(wxEVT_MOTION, &PayoffGrid::OnMotion, this);
  GetGridWindow()->Bind(wxEVT_LEAVE_WINDOW, &PayoffGrid::OnLeaveWindow, this);
}

void PayoffGrid::OnRangeSelecting(wxGridRangeSelectEvent &p_event)
{
  if (m_snappingSelection || !p_event.Selecting()) {
    p_event.Skip();
    return;
  }

  const int perContingency = m_table->GetPayoffColumnsPerContingency();
  const int leftCol = (p_event.GetLeftCol() / perContingency) * perContingency;
  const int rightCol = ((p_event.GetRightCol() / perContingency) + 1) * perContingency - 1;

  if (leftCol == p_event.GetLeftCol() && rightCol == p_event.GetRightCol()) {
    p_event.Skip();
    return;
  }

  // The raw range wxGrid is about to apply doesn't align to contingency boundaries --
  // veto it, and apply the snapped range ourselves instead. wxGrid re-derives the next
  // range from its own drag anchor and the pointer's current cell, not from whatever was
  // last actually selected, so this doesn't disturb the drag once it's under way.
  p_event.Veto();
  m_snappingSelection = true;
  SelectBlock(p_event.GetTopRow(), leftCol, p_event.GetBottomRow(), rightCol, true);
  m_snappingSelection = false;
}

void PayoffGrid::OnCellLeftClick(wxGridEvent &p_event)
{
  ClearSelection();
  TableGridBase::OnCellLeftClick(p_event);
}

std::vector<PureStrategyProfile> PayoffGrid::GetSelectedContingencies(int p_fallbackRow,
                                                                      int p_fallbackCol) const
{
  const int perContingency = m_table->GetPayoffColumnsPerContingency();
  std::set<std::pair<int, int>> blocks; // (row, contingency's column-block index)

  const wxGridCellCoordsArray topLeft = GetSelectionBlockTopLeft();
  const wxGridCellCoordsArray bottomRight = GetSelectionBlockBottomRight();
  for (size_t i = 0; i < topLeft.size(); ++i) {
    for (int row = topLeft[i].GetRow(); row <= bottomRight[i].GetRow(); ++row) {
      for (int col = topLeft[i].GetCol(); col <= bottomRight[i].GetCol(); ++col) {
        blocks.emplace(row, col / perContingency);
      }
    }
  }
  const wxGridCellCoordsArray selectedCells = GetSelectedCells();
  for (size_t i = 0; i < selectedCells.size(); ++i) {
    blocks.emplace(selectedCells[i].GetRow(), selectedCells[i].GetCol() / perContingency);
  }
  if (blocks.empty()) {
    blocks.emplace(p_fallbackRow, p_fallbackCol / perContingency);
  }

  std::vector<PureStrategyProfile> profiles;
  profiles.reserve(blocks.size());
  for (const auto &[row, colBlock] : blocks) {
    profiles.push_back(m_table->GetPayoffProfile(row, colBlock * perContingency));
  }
  return profiles;
}

GameOutcome PayoffGrid::GetSharedOutcome(const std::vector<PureStrategyProfile> &p_profiles)
{
  GameOutcome shared = p_profiles.front()->GetOutcome();
  for (const auto &profile : p_profiles) {
    if (profile->GetOutcome() != shared) {
      return GameOutcome();
    }
  }
  return (shared && !shared->IsNull()) ? shared : GameOutcome();
}

void PayoffGrid::ShowSetOutcomeDialog(const std::vector<PureStrategyProfile> &p_profiles)
{
  const Game game = m_table->GetDocument()->GetGame();

  // Does the selection already exactly match one existing outcome's membership?  If so,
  // this is really an in-place edit of that outcome, not a merge or split.
  const GameOutcome shared = GetSharedOutcome(p_profiles);

  wxString label;
  std::vector<wxString> payoffValues;
  std::set<const GameOutcomeRep *> ignoreForLabel;
  if (shared) {
    label = wxString::FromUTF8(shared->GetLabel());
    ignoreForLabel.insert(shared.get());
    for (const auto &player : game->GetPlayers()) {
      payoffValues.push_back(wxString::FromUTF8(shared->GetPayoff<std::string>(player)));
    }
  }
  else {
    label = wxString::FromUTF8(GenerateOutcomeLabel(game));
    const GameOutcome seed = p_profiles.front()->GetOutcome();
    for (const auto &player : game->GetPlayers()) {
      payoffValues.push_back(wxString::FromUTF8(seed->GetPayoff<std::string>(player)));
    }
  }

  SetOutcomeDialog dialog(this, game,
                          p_profiles.size() > 1 ? _("Merge into one outcome") : _("Set outcome"),
                          label, payoffValues, ignoreForLabel);
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }
  m_table->GetDocument()->DoMakeOutcome(p_profiles, dialog.GetPayoffs(),
                                        dialog.GetOutcomeLabel().ToStdString());
  ClearSelection();
}

void PayoffGrid::ShowEditOutcomeDialog(const GameOutcome &p_outcome)
{
  const Game game = m_table->GetDocument()->GetGame();
  std::vector<wxString> payoffValues;
  for (const auto &player : game->GetPlayers()) {
    payoffValues.push_back(wxString::FromUTF8(p_outcome->GetPayoff<std::string>(player)));
  }
  SetOutcomeDialog dialog(this, game, _("Edit outcome"), wxString::FromUTF8(p_outcome->GetLabel()),
                          payoffValues, {p_outcome.get()});
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }
  std::vector<wxString> payoffs;
  for (const auto &value : dialog.GetPayoffs()) {
    payoffs.push_back(wxString::FromUTF8(lexical_cast<std::string>(value)));
  }
  m_table->GetDocument()->DoSetOutcomeData(p_outcome, dialog.GetOutcomeLabel(), payoffs);
  ClearSelection();
}

void PayoffGrid::OnCellRightClick(wxGridEvent &p_event)
{
  const std::vector<PureStrategyProfile> profiles =
      GetSelectedContingencies(p_event.GetRow(), p_event.GetCol());

  const bool anyHasOutcome =
      std::any_of(profiles.begin(), profiles.end(), [](const PureStrategyProfile &p_profile) {
        return !p_profile->GetOutcome()->IsNull();
      });
  const GameOutcome shared = GetSharedOutcome(profiles);

  const int setOutcomeId = wxWindow::NewControlId();
  const int editOutcomeId = wxWindow::NewControlId();
  const int removeOutcomeId = wxWindow::NewControlId();

  wxMenu menu;
  menu.Append(setOutcomeId,
              profiles.size() > 1 ? _("Merge into one outcome...") : _("Set outcome..."));
  menu.Append(editOutcomeId, _("Edit outcome..."));
  menu.Enable(editOutcomeId, static_cast<bool>(shared));
  menu.Append(removeOutcomeId, _("Remove outcome"));
  menu.Enable(removeOutcomeId, anyHasOutcome);

  const int selection = GetPopupMenuSelectionFromUser(menu, p_event.GetPosition());

  try {
    if (selection == setOutcomeId) {
      ShowSetOutcomeDialog(profiles);
    }
    else if (selection == editOutcomeId) {
      ShowEditOutcomeDialog(shared);
    }
    else if (selection == removeOutcomeId) {
      for (const auto &profile : profiles) {
        m_table->GetDocument()->DoRemoveOutcome(profile);
      }
      ClearSelection();
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

void PayoffGrid::OnMotion(wxMouseEvent &p_event)
{
  // While a cell is being edited, the highlight is pinned to whatever outcome it showed
  // when editing began -- the mouse may wander (e.g. to check another part of the table)
  // without that being taken as "now hovering something else".
  if (IsCellEditControlShown()) {
    p_event.Skip();
    return;
  }
  const wxGridCellCoords coords = XYToCell(p_event.GetPosition());
  if (coords.GetRow() >= 0 && coords.GetCol() >= 0) {
    m_table->SetHoverOutcome(
        m_table->GetPayoffProfile(coords.GetRow(), coords.GetCol())->GetOutcome());
  }
  else {
    m_table->SetHoverOutcome(GameOutcome());
  }
  p_event.Skip();
}

void PayoffGrid::OnLeaveWindow(wxMouseEvent &p_event)
{
  if (IsCellEditControlShown()) {
    p_event.Skip();
    return;
  }
  // A cell editor is a real child window created directly over the current cell (by
  // EnableCellEditControl(), including the deferred one MoveEditorByTab() creates for
  // the next cell); on at least macOS/Cocoa, that can itself generate a leave-window
  // event on the grid window underneath even though the pointer never moved. Only
  // clear the highlight if the pointer has genuinely left the grid's screen area.
  const wxRect windowRect(GetGridWindow()->GetScreenPosition(), GetGridWindow()->GetSize());
  if (!windowRect.Contains(wxGetMousePosition())) {
    m_table->SetHoverOutcome(GameOutcome());
  }
  p_event.Skip();
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
