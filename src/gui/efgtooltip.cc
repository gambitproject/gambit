//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgtooltip.cc
// Popups for editing a node's outcome, appending a move at a node, and for showing hover
// information
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
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/popupwin.h>
#include <wx/spinctrl.h>
#include <wx/scrolwin.h>
#include <wx/statbmp.h>
#include <wx/statline.h>

#include "games.h"

#include "editlabel.h"
#include "efgdisplay.h"
#include "valnumber.h"

namespace Gambit::GUI {

class OutcomeEditorPopup : public wxPopupTransientWindow {
public:
  OutcomeEditorPopup(EfgDisplay *p_owner, const std::shared_ptr<GameDocument> &p_doc);

  void BeginEdit(const GameNode &p_node, int p_initialPlayer = 0);
  bool Commit();
  void Cancel();

protected:
  void OnDismiss() override;

private:
  struct ValidationResult {
    bool ok{true};
    wxString message;
    wxTextCtrl *ctrl{nullptr};
  };

  void BuildControls();
  void LoadValues();
  void PositionPopup();
  void OnKeyDown(wxKeyEvent &p_event);

  ValidationResult ValidatePayoffs(std::vector<wxString> &p_payoffs);
  void ShowValidationFailure(const wxString &p_message, wxTextCtrl *p_ctrl);
  void ClearValidationFailure();
  void RestoreAfterFailedCommit(wxTextCtrl *p_invalidCtrl);

  EfgDisplay *m_owner;
  std::shared_ptr<GameDocument> m_doc;

  GameNode m_node;

  wxPanel *m_contentPanel;
  wxTextCtrl *m_labelCtrl;
  wxStaticText *m_errorText;
  wxFlexGridSizer *m_gridSizer;
  std::vector<wxTextCtrl *> m_payoffCtrls;

  int m_initialPlayer{0};
  bool m_cancelled{false};
  bool m_dismissing{false};
  bool m_committing{false};
  bool m_restoringAfterFailedCommit{false};
};

OutcomeEditorPopup::OutcomeEditorPopup(EfgDisplay *p_owner,
                                       const std::shared_ptr<GameDocument> &p_doc)
  : wxPopupTransientWindow(p_owner, wxBORDER_NONE), m_owner(p_owner), m_doc(p_doc),
    m_contentPanel(nullptr), m_labelCtrl(nullptr), m_errorText(nullptr)
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));

  BuildControls();

  Bind(wxEVT_CHAR_HOOK, &OutcomeEditorPopup::OnKeyDown, this);
}

void OutcomeEditorPopup::BuildControls()
{
  auto *popupSizer = new wxBoxSizer(wxVERTICAL);

  m_contentPanel = new wxPanel(this);
  m_contentPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

  auto *outerSizer = new wxBoxSizer(wxVERTICAL);

  auto *heading = new wxStaticText(m_contentPanel, wxID_ANY, _("Outcome"));

  wxFont headingFont = heading->GetFont();
  headingFont.SetWeight(wxFONTWEIGHT_BOLD);
  headingFont.SetPointSize(headingFont.GetPointSize() + 1);
  heading->SetFont(headingFont);

  outerSizer->Add(heading, 0, wxLEFT | wxRIGHT | wxTOP, FromDIP(12));

  auto *labelSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(12));
  labelSizer->AddGrowableCol(1, 1);

  labelSizer->Add(new wxStaticText(m_contentPanel, wxID_ANY, _("Label")), 0,
                  wxALIGN_CENTER_VERTICAL);

  m_labelCtrl = new wxTextCtrl(m_contentPanel, wxID_ANY);
  m_labelCtrl->SetMinSize(wxSize(FromDIP(180), -1));
  labelSizer->Add(m_labelCtrl, 1, wxEXPAND);

  outerSizer->Add(labelSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(12));

  auto *payoffHeading = new wxStaticText(m_contentPanel, wxID_ANY, _("Payoffs"));

  wxFont payoffHeadingFont = payoffHeading->GetFont();
  payoffHeadingFont.SetWeight(wxFONTWEIGHT_BOLD);
  payoffHeading->SetFont(payoffHeadingFont);

  outerSizer->Add(payoffHeading, 0, wxLEFT | wxRIGHT | wxTOP, FromDIP(12));

  auto *payoffSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(12));
  payoffSizer->AddGrowableCol(1, 1);

  const Game game = m_doc->GetGame();

  for (size_t player = 1; player <= m_doc->GetGame()->NumPlayers(); ++player) {
    const GamePlayer gamePlayer = game->GetPlayer(player);

    payoffSizer->Add(
        new wxStaticText(m_contentPanel, wxID_ANY, wxString::FromUTF8(gamePlayer->GetLabel())), 0,
        wxALIGN_CENTER_VERTICAL);

    auto *payoffCtrl = new wxTextCtrl(m_contentPanel, wxID_ANY);

    payoffCtrl->SetValidator(NumberValidator(nullptr));
    payoffCtrl->SetMinSize(wxSize(FromDIP(100), -1));

    payoffSizer->Add(payoffCtrl, 1, wxEXPAND);
    m_payoffCtrls.push_back(payoffCtrl);
  }

  outerSizer->Add(payoffSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, FromDIP(12));

  m_errorText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
  m_errorText->SetForegroundColour(*wxRED);
  m_errorText->Wrap(FromDIP(260));
  m_errorText->Hide();

  outerSizer->Add(m_errorText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(12));

  m_contentPanel->SetSizer(outerSizer);

  popupSizer->Add(m_contentPanel, 1, wxEXPAND | wxALL, FromDIP(1));

  SetSizerAndFit(popupSizer);
}

void OutcomeEditorPopup::LoadValues()
{
  const GameOutcome outcome = m_node ? m_node->GetOutcome() : nullptr;

  if (!outcome) {
    m_labelCtrl->Clear();

    for (auto *ctrl : m_payoffCtrls) {
      ctrl->SetValue(wxT("0"));
    }

    return;
  }

  m_labelCtrl->SetValue(wxString::FromUTF8(outcome->GetLabel()));

  const Game game = m_doc->GetGame();

  for (size_t player = 1; player <= m_payoffCtrls.size(); ++player) {
    const std::string payoff = outcome->GetPayoff<std::string>(game->GetPlayer(player));

    m_payoffCtrls[player - 1]->SetValue(wxString(payoff.c_str(), *wxConvCurrent));
  }
}

void OutcomeEditorPopup::PositionPopup()
{
  auto entry = m_owner->GetLayout().GetNodeEntry(m_node);
  if (!entry) {
    return;
  }

  int clientX, clientY;
  m_owner->CalcScrolledPosition(m_owner->LayoutToDevice(entry->GetX() + 20),
                                m_owner->LayoutToDevice(entry->GetY()), &clientX, &clientY);

  const wxPoint screenPoint = m_owner->ClientToScreen(wxPoint(clientX, clientY));

  Position(screenPoint, wxSize(FromDIP(8), FromDIP(8)));
}

void OutcomeEditorPopup::BeginEdit(const GameNode &p_node, int p_initialPlayer)
{
  m_node = p_node;
  m_initialPlayer = p_initialPlayer;
  m_cancelled = false;
  m_dismissing = false;
  m_committing = false;
  m_restoringAfterFailedCommit = false;

  LoadValues();
  ClearValidationFailure();
  Fit();
  PositionPopup();

  Popup();

  if (m_initialPlayer > 0 && m_initialPlayer <= static_cast<int>(m_payoffCtrls.size())) {
    wxTextCtrl *ctrl = m_payoffCtrls[m_initialPlayer - 1];
    ctrl->SetFocus();
    ctrl->SelectAll();
  }
  else {
    m_labelCtrl->SetFocus();
    m_labelCtrl->SetInsertionPointEnd();
  }
}

void OutcomeEditorPopup::OnDismiss()
{
  if (m_dismissing || m_restoringAfterFailedCommit) {
    return;
  }

  if (m_cancelled) {
    m_cancelled = false;
    m_node = nullptr;
    return;
  }

  Commit();
}

void OutcomeEditorPopup::OnKeyDown(wxKeyEvent &p_event)
{
  switch (p_event.GetKeyCode()) {
  case WXK_ESCAPE:
    Cancel();
    return;

  case WXK_RETURN:
  case WXK_NUMPAD_ENTER:
    Commit();
    return;

  default:
    p_event.Skip();
  }
}

void OutcomeEditorPopup::Cancel()
{
  if (!m_node) {
    return;
  }

  ClearValidationFailure();

  m_cancelled = true;
  Dismiss();
}

OutcomeEditorPopup::ValidationResult
OutcomeEditorPopup::ValidatePayoffs(std::vector<wxString> &p_payoffs)
{
  p_payoffs.clear();
  p_payoffs.reserve(m_payoffCtrls.size());

  for (size_t player = 1; player <= m_payoffCtrls.size(); ++player) {
    wxTextCtrl *ctrl = m_payoffCtrls[player - 1];
    wxString value = ctrl->GetValue();

    if (value.EndsWith(wxT("/"))) {
      value.RemoveLast();
      ctrl->SetValue(value);
      ctrl->SetInsertionPointEnd();
    }

    try {
      lexical_cast<Rational>(value.ToStdString());
    }
    catch (const std::exception &) {
      return {false,
              wxString::Format(_("Payoff for player %lu is not a valid number."),
                               static_cast<unsigned long>(player)),
              ctrl};
    }

    p_payoffs.push_back(value);
  }

  return {};
}

bool OutcomeEditorPopup::Commit()
{
  if (!m_node || m_committing) {
    return false;
  }

  m_committing = true;

  std::vector<wxString> payoffs;
  const ValidationResult validation = ValidatePayoffs(payoffs);

  if (!validation.ok) {
    ShowValidationFailure(validation.message, validation.ctrl);
    RestoreAfterFailedCommit(validation.ctrl);
    m_committing = false;
    return false;
  }

  try {
    m_doc->DoSetOutcomeData(m_node, m_labelCtrl->GetValue(), payoffs);
  }
  catch (const std::exception &ex) {
    ShowValidationFailure(wxString::FromUTF8(ex.what()), m_labelCtrl);
    RestoreAfterFailedCommit(m_labelCtrl);
    m_committing = false;
    return false;
  }

  ClearValidationFailure();

  m_dismissing = true;
  Dismiss();
  m_dismissing = false;
  m_node = nullptr;

  m_committing = false;
  return true;
}

void OutcomeEditorPopup::ShowValidationFailure(const wxString &p_message, wxTextCtrl *p_ctrl)
{
  wxBell();

  if (m_errorText) {
    m_errorText->SetLabel(p_message);
    m_errorText->Wrap(FromDIP(260));
    m_errorText->Show();
  }

  if (m_contentPanel) {
    m_contentPanel->Layout();
  }

  Fit();
  PositionPopup();

  if (p_ctrl) {
    p_ctrl->SetFocus();
    p_ctrl->SelectAll();
  }
}

void OutcomeEditorPopup::ClearValidationFailure()
{
  if (!m_errorText) {
    return;
  }

  m_errorText->SetLabel(wxEmptyString);
  m_errorText->Hide();

  if (m_contentPanel) {
    m_contentPanel->Layout();
  }
}

void OutcomeEditorPopup::RestoreAfterFailedCommit(wxTextCtrl *p_invalidCtrl)
{
  if (m_restoringAfterFailedCommit) {
    return;
  }

  m_restoringAfterFailedCommit = true;

  CallAfter([this, p_invalidCtrl]() {
    m_restoringAfterFailedCommit = false;

    if (!m_node) {
      return;
    }

    PositionPopup();

    if (!IsShown()) {
      Popup();
    }

    wxTextCtrl *ctrl = p_invalidCtrl ? p_invalidCtrl : m_labelCtrl;
    ctrl->SetFocus();
    ctrl->SelectAll();
  });
}
//--------------------------------------------------------------------------
//                        class AppendMovePopup
//--------------------------------------------------------------------------

namespace {
const wxColour kInvalidLabelBg(255, 220, 220);
// Logical (pre-FromDIP) cap on the action-list scroller's height, past which it scrolls
// instead of growing the dialog further -- roughly six rows at the row height this grid uses.
constexpr int kMaxGridHeightDIP = 200;
} // namespace

// Floating window for appending a fresh move at a terminal node, opened as soon as a player is
// picked from the node menu's "Insert move for"/"Assign this move to" submenu (efgnodemenu.cc).
// Grows its action-label list either by tabbing off the last row (which appends a new one) or
// via the count field; the two are kept in sync. Enter accepts the panel from any field;
// commits the same way on losing activation (clicking elsewhere); Escape cancels.
//
// This started out as a wxPopupTransientWindow (matching OutcomeEditorPopup), then a plain
// wxFrame with wxNO_BORDER, then a wxMiniFrame with a tiny caption -- none of the three ever
// reliably received real keyboard focus on macOS (typing produced the system's unhandled-key
// beep or nothing at all; Escape, bound at the window's own top level, had no effect either),
// regardless of how Show()/SetFocus() were sequenced. Every one of those is a window class this
// app had never actually shown non-modally before. wxDialog is the one class already proven,
// everywhere else in this app, to reliably take keyboard focus for its children -- every other
// dialog in src/gui relies on exactly that. The only twist here is calling Show() instead of
// ShowModal(), which changes whether the parent is blocked, not whether the window can become
// key -- that's a property of being an activatable top-level window, not of modality.
class AppendMovePopup : public wxDialog {
public:
  AppendMovePopup(EfgDisplay *p_owner, const std::shared_ptr<GameDocument> &p_doc);

  void BeginAppend(const GameNode &p_node, const GamePlayer &p_player);
  bool Commit();
  void Cancel();

private:
  struct Row {
    wxString label;
    wxString prob; // only meaningful when m_isChance
    wxTextCtrl *labelCtrl{nullptr};
    wxTextCtrl *probCtrl{nullptr};
  };

  void BuildControls();
  // Rebuilds the row grid's controls from m_rows -- called whenever the row count changes.
  void RebuildGrid();
  // Snapshots whatever's typed in the live controls into m_rows, then grows or shrinks m_rows
  // to p_count (new rows get a numbered placeholder label and, for chance, a recomputed
  // uniform probability) and rebuilds. Shrinking trims from the bottom unconditionally, even
  // if a trimmed row already had a typed label -- this is all pre-commit, so there's nothing
  // to preserve carefully.
  void SetRowCount(int p_count);
  void PositionPopup();
  void OnKeyDown(wxKeyEvent &p_event);
  // Fires when this window gains or loses activation -- losing it (the user clicked the tree
  // canvas, another window, or another app) commits, the same way OutcomeEditorPopup commits
  // on OnDismiss(). Gaining activation is ignored.
  void OnActivate(wxActivateEvent &p_event);
  // Commits, unless this deactivation is the one Cancel()/Commit() themselves triggered by
  // hiding the window (m_dismissing/m_restoringAfterFailedCommit) or the user just cancelled
  // (m_cancelled).
  void OnDismiss();
  // BeginAppend is called from OnSetPlayerMenu (efgnodemenu.cc), itself a wxMenu item handler
  // still running while the menu's own native close sequence is unwinding, which ends by
  // restoring activation to the tree canvas; showing and focusing this window synchronously
  // there can still lose that race. A short one-shot timer reliably lands after it's done.
  void OnOpenTimer(wxTimerEvent &p_event);
  // Bound to every row field. Tab falling off the grid's true last field grows the list by one
  // row and focuses it; every other field's Tab is left to native focus traversal, which
  // already does the right thing. Enter is handled globally instead (see OnKeyDown) -- it
  // accepts the whole panel regardless of which field it's pressed in.
  void OnFieldKeyDown(wxKeyEvent &p_event, wxTextCtrl *p_ctrl);
  void OnCountChanged(wxCommandEvent &p_event);

  // Checks every row's label for being empty or a duplicate of another row's, colouring each
  // offending field's background (like EditMoveDialog's ActionPanel) and returning a
  // description of the first problem found, or an empty string if all are valid. A pure check
  // would leave the fields uncoloured until a commit was attempted; this runs on every
  // keystroke instead (see UpdateValidation) so the highlighting is never stale.
  wxString ValidateLabels();
  // Chance moves only. Parses every row's probability field into p_probs and checks the
  // resulting distribution (via ValidateDistribution), colouring offending fields the same way
  // ValidateLabels() does and returning a description of the first problem found, or an empty
  // string if the distribution is valid -- in which case p_probs holds the parsed values.
  // Leaves p_probs empty on any failure.
  wxString ValidateProbabilities(std::vector<Number> &p_probs);
  // Refreshes the error text/highlighting from ValidateLabels() (and, for chance moves,
  // ValidateProbabilities()) -- called after every row edit and every row-count change, not
  // just when a commit is attempted, so an invalid label or probability is flagged immediately
  // rather than only when the user tries to leave.
  void UpdateValidation();
  void ShowValidationFailure(const wxString &p_message, wxTextCtrl *p_ctrl);
  void ClearValidationFailure();
  void RestoreAfterFailedCommit(wxTextCtrl *p_invalidCtrl);

  EfgDisplay *m_owner;
  std::shared_ptr<GameDocument> m_doc;

  GameNode m_node;
  GamePlayer m_player;
  bool m_isChance{false};

  wxPanel *m_contentPanel;
  // Shaded strip holding the "Append move for <player> with <count> actions" sentence,
  // divided from the action list below by a wxStaticLine (see BuildControls).
  wxPanel *m_headerPanel;
  wxStaticBitmap *m_playerSwatch;
  wxStaticText *m_playerNameText;
  wxSpinCtrl *m_countCtrl;
  // Scrolls once the row list grows past kMaxGridHeight, rather than growing the dialog (and
  // the screen it's anchored to) without bound.
  wxScrolledWindow *m_gridScroller{nullptr};
  wxBoxSizer *m_gridContainer{nullptr};
  wxStaticText *m_hintText;
  wxStaticText *m_errorText;
  std::vector<Row> m_rows;
  // A freshly created LabelTextCtrl's background, captured the first time RebuildGrid creates
  // one; the colour ValidateLabels() restores a row to once it's no longer empty/duplicate.
  wxColour m_defaultLabelBg;
  wxTimer m_openTimer;

  bool m_cancelled{false};
  bool m_dismissing{false};
  bool m_committing{false};
  bool m_restoringAfterFailedCommit{false};
};

AppendMovePopup::AppendMovePopup(EfgDisplay *p_owner, const std::shared_ptr<GameDocument> &p_doc)
  : wxDialog(p_owner, wxID_ANY, _("Append move"), wxDefaultPosition, wxDefaultSize, wxCAPTION),
    m_owner(p_owner), m_doc(p_doc), m_contentPanel(nullptr), m_headerPanel(nullptr),
    m_playerSwatch(nullptr), m_playerNameText(nullptr), m_countCtrl(nullptr), m_hintText(nullptr),
    m_errorText(nullptr)
{
  BuildControls();

  Bind(wxEVT_CHAR_HOOK, &AppendMovePopup::OnKeyDown, this);
  Bind(wxEVT_ACTIVATE, &AppendMovePopup::OnActivate, this);

  m_openTimer.SetOwner(this);
  Bind(wxEVT_TIMER, &AppendMovePopup::OnOpenTimer, this);
}

void AppendMovePopup::BuildControls()
{
  m_contentPanel = new wxPanel(this);
  m_contentPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

  auto *outerSizer = new wxBoxSizer(wxVERTICAL);

  // Header: a shaded strip, divided from the action list below, whose contents read as one
  // sentence -- "Append move for <swatch> <player> with <count> actions" -- rather than as
  // separate label/field rows.
  m_headerPanel = new wxPanel(m_contentPanel);
  m_headerPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

  // A plain (non-wrapping) horizontal sizer, deliberately: wxWrapSizer's reported minimum
  // width is just enough for its widest single item, since it exists to shrink onto multiple
  // lines. That meant this sentence's wrap point tracked whatever width the row grid happened
  // to need, not the sentence's own content -- unpredictable as the player name or count digit
  // width changed. A non-wrapping sizer reports its true, one-line width as its minimum, so
  // Fit() below sizes the whole dialog to comfortably fit the sentence on one line every time.
  auto *sentenceSizer = new wxBoxSizer(wxHORIZONTAL);
  sentenceSizer->Add(new wxStaticText(m_headerPanel, wxID_ANY, _("Append move for")), 0,
                     wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(5));

  m_playerSwatch = new wxStaticBitmap(m_headerPanel, wxID_ANY, wxNullBitmap);
  sentenceSizer->Add(m_playerSwatch, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(5));

  m_playerNameText = new wxStaticText(m_headerPanel, wxID_ANY, wxEmptyString);
  wxFont nameFont = m_playerNameText->GetFont();
  nameFont.SetWeight(wxFONTWEIGHT_BOLD);
  m_playerNameText->SetFont(nameFont);
  sentenceSizer->Add(m_playerNameText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(5));

  sentenceSizer->Add(new wxStaticText(m_headerPanel, wxID_ANY, _("with")), 0,
                     wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(5));

  m_countCtrl = new wxSpinCtrl(m_headerPanel, wxID_ANY, wxT("2"), wxDefaultPosition,
                               wxSize(FromDIP(55), -1), wxSP_ARROW_KEYS, 1, 10000, 2);
  sentenceSizer->Add(m_countCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(5));

  sentenceSizer->Add(new wxStaticText(m_headerPanel, wxID_ANY, _("actions")), 0,
                     wxALIGN_CENTER_VERTICAL);

  m_countCtrl->Bind(wxEVT_SPINCTRL, &AppendMovePopup::OnCountChanged, this);
  m_countCtrl->Bind(wxEVT_TEXT_ENTER, &AppendMovePopup::OnCountChanged, this);
  m_countCtrl->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent &p_event) {
    SetRowCount(m_countCtrl->GetValue());
    p_event.Skip();
  });

  auto *headerBorderSizer = new wxBoxSizer(wxVERTICAL);
  headerBorderSizer->Add(sentenceSizer, 0, wxEXPAND | wxALL, FromDIP(12));
  m_headerPanel->SetSizer(headerBorderSizer);
  outerSizer->Add(m_headerPanel, 0, wxEXPAND);

  outerSizer->Add(new wxStaticLine(m_contentPanel), 0, wxEXPAND);

  m_gridScroller = new wxScrolledWindow(m_contentPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxVSCROLL | wxTAB_TRAVERSAL);
  m_gridScroller->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
  m_gridScroller->SetScrollRate(0, FromDIP(10));
  m_gridContainer = new wxBoxSizer(wxVERTICAL);
  m_gridScroller->SetSizer(m_gridContainer);
  outerSizer->Add(m_gridScroller, 1, wxEXPAND | wxALL, FromDIP(12));

  outerSizer->Add(new wxStaticLine(m_contentPanel), 0, wxEXPAND);

  m_hintText =
      new wxStaticText(m_contentPanel, wxID_ANY, _("Tab past the last action to add another"));
  m_hintText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
  wxFont hintFont = m_hintText->GetFont();
  hintFont.SetPointSize(hintFont.GetPointSize() - 1);
  m_hintText->SetFont(hintFont);
  outerSizer->Add(m_hintText, 0, wxALL, FromDIP(10));

  m_errorText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
  m_errorText->SetForegroundColour(*wxRED);
  m_errorText->Wrap(FromDIP(260));
  m_errorText->Hide();
  outerSizer->Add(m_errorText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(12));

  m_contentPanel->SetSizer(outerSizer);

  auto *frameSizer = new wxBoxSizer(wxVERTICAL);
  frameSizer->Add(m_contentPanel, 1, wxEXPAND);
  SetSizerAndFit(frameSizer);
}

void AppendMovePopup::RebuildGrid()
{
  m_gridContainer->Clear(true); // destroys the previous row controls

  const int numColumns = m_isChance ? 2 : 1;
  auto *gridSizer = new wxFlexGridSizer(numColumns, FromDIP(5), FromDIP(10));
  gridSizer->AddGrowableCol(0, 1);

  gridSizer->Add(new wxStaticText(m_gridScroller, wxID_STATIC, _("Label")), 0,
                 wxALIGN_CENTER_VERTICAL);
  if (m_isChance) {
    gridSizer->Add(new wxStaticText(m_gridScroller, wxID_STATIC, _("Probability")), 0,
                   wxALIGN_CENTER_VERTICAL);
  }

  for (size_t i = 0; i < m_rows.size(); i++) {
    Row &row = m_rows[i];
    const bool isLastRow = (i + 1 == m_rows.size());
    // Only the grid's true last field needs wxTE_PROCESS_TAB, so that a Tab press there
    // reaches OnFieldKeyDown instead of being consumed by native focus traversal (which is
    // what was making Tab silently jump back up to the count field). Every other field's Tab
    // already does the right thing -- moves to the next field -- without any help.
    const bool labelIsLastField = isLastRow && !m_isChance;

    row.labelCtrl = new LabelTextCtrl(m_gridScroller, wxID_ANY, row.label, wxDefaultPosition,
                                      wxDefaultSize, labelIsLastField ? wxTE_PROCESS_TAB : 0);
    row.labelCtrl->SetMinSize(wxSize(FromDIP(160), -1));
    if (i == 0) {
      m_defaultLabelBg = row.labelCtrl->GetBackgroundColour();
    }
    row.labelCtrl->Bind(wxEVT_TEXT, [this, i](wxCommandEvent &p_event) {
      if (i < m_rows.size()) {
        m_rows[i].label = m_rows[i].labelCtrl->GetValue();
      }
      UpdateValidation();
      p_event.Skip();
    });
    row.labelCtrl->Bind(wxEVT_KEY_DOWN, [this, ctrl = row.labelCtrl](wxKeyEvent &p_event) {
      OnFieldKeyDown(p_event, ctrl);
    });
    gridSizer->Add(row.labelCtrl, 1, wxEXPAND);

    if (m_isChance) {
      row.probCtrl = new wxTextCtrl(m_gridScroller, wxID_ANY, row.prob, wxDefaultPosition,
                                    wxDefaultSize, isLastRow ? wxTE_PROCESS_TAB : 0);
      row.probCtrl->SetMinSize(wxSize(FromDIP(80), -1));
      row.probCtrl->Bind(wxEVT_TEXT, [this, i](wxCommandEvent &p_event) {
        if (i < m_rows.size()) {
          m_rows[i].prob = m_rows[i].probCtrl->GetValue();
        }
        UpdateValidation();
        p_event.Skip();
      });
      row.probCtrl->Bind(wxEVT_KEY_DOWN, [this, ctrl = row.probCtrl](wxKeyEvent &p_event) {
        OnFieldKeyDown(p_event, ctrl);
      });
      gridSizer->Add(row.probCtrl, 0, wxEXPAND);
    }
  }

  m_gridContainer->Add(gridSizer, 1, wxEXPAND);

  // Recomputed on every rebuild, not just once: with the row count changing dynamically (unlike
  // EditMoveDialog's ActionPanel, which sets this once at construction and otherwise leaves
  // sizing to the user via wxRESIZE_BORDER), the popup should still shrink-to-fit for a
  // handful of rows and only clamp -- rather than keep growing -- once the list would push the
  // dialog past kMaxGridHeightDIP.
  const int naturalHeight = m_gridContainer->CalcMin().GetHeight();
  m_gridScroller->SetMinSize(wxSize(-1, std::min(naturalHeight, FromDIP(kMaxGridHeightDIP))));
  m_gridScroller->FitInside();

  m_contentPanel->Layout();
}

void AppendMovePopup::SetRowCount(int p_count)
{
  p_count = std::max(1, p_count);

  // Snapshot whatever's currently typed before the row controls (about to be rebuilt, or
  // trimmed away) are torn down.
  for (auto &row : m_rows) {
    if (row.labelCtrl) {
      row.label = row.labelCtrl->GetValue();
    }
    if (row.probCtrl) {
      row.prob = row.probCtrl->GetValue();
    }
  }

  const bool hadExistingRows = !m_rows.empty();

  // For a chance move, a newly added row's default probability is whatever's still needed to
  // bring the *existing* rows up to summing to one (or zero, if they already sum to one or
  // more) -- not a uniform recompute of every row, which would silently overwrite whatever was
  // already typed into rows that aren't changing. The one exception is populating the panel
  // for the first time (no existing rows to take a share from at all), which still defaults to
  // a uniform split, same as before.
  Rational remaining(1);
  if (m_isChance && hadExistingRows) {
    for (const auto &row : m_rows) {
      try {
        remaining -= static_cast<Rational>(Number(row.prob.ToStdString()));
      }
      catch (const std::exception &) {
        // Not parseable (yet) -- contributes nothing to the running sum, since there's no
        // sensible number to subtract.
      }
    }
  }

  if (p_count < static_cast<int>(m_rows.size())) {
    m_rows.resize(p_count); // trim from the bottom, unconditionally -- see SetRowCount's header
  }
  else {
    for (int i = static_cast<int>(m_rows.size()); i < p_count; i++) {
      Row row;
      row.label = wxString::Format(wxT("%d"), i + 1);
      if (m_isChance && hadExistingRows) {
        const Rational share = (remaining > Rational(0)) ? remaining : Rational(0);
        row.prob = wxString::FromUTF8(static_cast<std::string>(Number(share)));
        remaining -= share;
      }
      m_rows.push_back(std::move(row));
    }

    if (m_isChance && !hadExistingRows) {
      const wxString uniform = wxString::FromUTF8(
          static_cast<std::string>(Number(Rational(1, static_cast<int>(m_rows.size())))));
      for (auto &row : m_rows) {
        row.prob = uniform;
      }
    }
  }

  RebuildGrid();

  if (m_countCtrl->GetValue() != p_count) {
    m_countCtrl->SetValue(p_count);
  }

  UpdateValidation(); // also does the Fit()/PositionPopup() a plain row-count change still needs
}

void AppendMovePopup::OnCountChanged(wxCommandEvent &p_event)
{
  SetRowCount(m_countCtrl->GetValue());
  p_event.Skip();
}

void AppendMovePopup::OnFieldKeyDown(wxKeyEvent &p_event, wxTextCtrl *p_ctrl)
{
  // Enter now accepts the whole panel (see OnKeyDown) rather than acting per-field, so this
  // only needs to handle Tab falling off the grid's true last field.
  if (p_event.GetKeyCode() == WXK_TAB) {
    wxTextCtrl *const lastField = m_isChance ? m_rows.back().probCtrl : m_rows.back().labelCtrl;
    if (p_ctrl == lastField) {
      SetRowCount(static_cast<int>(m_rows.size()) + 1);
      m_rows.back().labelCtrl->SetFocus();
      m_rows.back().labelCtrl->SelectAll();
      return;
    }
  }
  p_event.Skip(); // not the grid's last field -- native Tab traversal already does the right thing
}

void AppendMovePopup::PositionPopup()
{
  auto entry = m_owner->GetLayout().GetNodeEntry(m_node);
  if (!entry) {
    return;
  }

  int clientX, clientY;
  m_owner->CalcScrolledPosition(m_owner->LayoutToDevice(entry->GetX() + 20),
                                m_owner->LayoutToDevice(entry->GetY()), &clientX, &clientY);

  const wxPoint screenPoint = m_owner->ClientToScreen(wxPoint(clientX, clientY));

  // wxFrame has no wxPopupWindowBase-style Position() that avoids screen edges; a plain
  // SetPosition() is fine here since the popup opens just below/right of the node like before.
  SetPosition(screenPoint);
}

void AppendMovePopup::BeginAppend(const GameNode &p_node, const GamePlayer &p_player)
{
  m_node = p_node;
  m_player = p_player;
  m_isChance = p_player->IsChance();
  m_cancelled = false;
  m_dismissing = false;
  m_committing = false;
  m_restoringAfterFailedCommit = false;

  wxString label = wxString::FromUTF8(p_player->GetLabel());
  if (label.empty()) {
    label = m_isChance ? wxString(_("Chance"))
                       : wxString::Format(_("Player %d"), p_player->GetNumber());
  }
  m_playerNameText->SetLabel(label);
  m_playerSwatch->SetBitmap(MakeColorSwatch(m_doc->GetStyle().GetPlayerColor(p_player)));
  m_headerPanel->Layout();

  m_rows.clear();
  SetRowCount(2); // also runs UpdateValidation(), which clears any stale error/highlighting

  m_contentPanel->Layout();
  Fit();
  PositionPopup();

  // See OnOpenTimer's declaration for why Show()/SetFocus() are deferred via a timer rather
  // than done directly here.
  m_openTimer.StartOnce(60);
}

void AppendMovePopup::OnOpenTimer(wxTimerEvent &)
{
  if (!m_node) {
    return; // superseded by a Cancel()/Commit() (or another BeginAppend) before this fired
  }
  Show();
  m_rows[0].labelCtrl->SetFocus();
  m_rows[0].labelCtrl->SelectAll();
}

void AppendMovePopup::OnActivate(wxActivateEvent &p_event)
{
  if (!p_event.GetActive()) {
    OnDismiss();
  }
  p_event.Skip();
}

void AppendMovePopup::OnDismiss()
{
  if (m_dismissing || m_restoringAfterFailedCommit) {
    return;
  }

  if (m_cancelled) {
    m_cancelled = false;
    m_node = nullptr;
    return;
  }

  Commit();
}

void AppendMovePopup::OnKeyDown(wxKeyEvent &p_event)
{
  switch (p_event.GetKeyCode()) {
  case WXK_ESCAPE:
    Cancel();
    return;

  case WXK_RETURN:
  case WXK_NUMPAD_ENTER:
    // Accepts the panel regardless of which field has focus -- Tab (OnFieldKeyDown) is the
    // one that grows the list; Enter always means "I'm done."
    Commit();
    return;

  default:
    p_event.Skip();
  }
}

void AppendMovePopup::Cancel()
{
  if (!m_node) {
    return;
  }

  ClearValidationFailure();

  m_cancelled = true;
  Hide();
}

wxString AppendMovePopup::ValidateLabels()
{
  wxString message;
  for (size_t i = 0; i < m_rows.size(); i++) {
    const wxString value = m_rows[i].labelCtrl->GetValue();
    bool invalid = value.empty();
    for (size_t j = 0; !invalid && j < m_rows.size(); j++) {
      if (j != i && m_rows[j].labelCtrl->GetValue() == value) {
        invalid = true;
      }
    }

    m_rows[i].labelCtrl->SetBackgroundColour(invalid ? kInvalidLabelBg : m_defaultLabelBg);
    m_rows[i].labelCtrl->Refresh();

    if (invalid && message.empty()) {
      message = value.empty() ? _("Action labels cannot be empty.")
                              : _("Action labels must be unique within the information set.");
    }
  }
  return message;
}

wxString AppendMovePopup::ValidateProbabilities(std::vector<Number> &p_probs)
{
  p_probs.clear();

  wxString message;
  bool allParsed = true;
  for (auto &row : m_rows) {
    bool invalid = false;
    try {
      p_probs.emplace_back(row.probCtrl->GetValue().ToStdString());
    }
    catch (const std::exception &) {
      invalid = true;
      allParsed = false;
    }
    row.probCtrl->SetBackgroundColour(invalid ? kInvalidLabelBg : m_defaultLabelBg);
    row.probCtrl->Refresh();
    if (invalid && message.empty()) {
      message = _("Probabilities must be valid numbers.");
    }
  }

  if (!allParsed) {
    p_probs.clear();
    return message;
  }

  try {
    ValidateDistribution(p_probs);
  }
  catch (const std::exception &) {
    for (auto &row : m_rows) {
      row.probCtrl->SetBackgroundColour(kInvalidLabelBg);
      row.probCtrl->Refresh();
    }
    p_probs.clear();
    return _("Probabilities must be nonnegative numbers summing to one.");
  }

  return wxEmptyString;
}

void AppendMovePopup::UpdateValidation()
{
  wxString message = ValidateLabels();
  if (m_isChance) {
    std::vector<Number> probs;
    const wxString probError = ValidateProbabilities(probs);
    if (message.empty()) {
      message = probError;
    }
  }
  if (message.empty()) {
    ClearValidationFailure();
  }
  else {
    m_errorText->SetLabel(message);
    m_errorText->Wrap(FromDIP(260));
    m_errorText->Show();
    m_contentPanel->Layout();
  }
  Fit();
  PositionPopup();
}

bool AppendMovePopup::Commit()
{
  if (!m_node || m_committing) {
    return false;
  }

  m_committing = true;

  UpdateValidation();
  if (!m_errorText->GetLabel().empty()) {
    wxBell();
    wxTextCtrl *ctrl = m_rows.empty() ? nullptr : m_rows.front().labelCtrl;
    RestoreAfterFailedCommit(ctrl);
    m_committing = false;
    return false;
  }

  std::vector<std::string> labels;
  for (auto &row : m_rows) {
    labels.push_back(row.labelCtrl->GetValue().ToStdString(wxConvUTF8));
  }

  std::vector<Number> probs;
  if (m_isChance) {
    // UpdateValidation() above already ran this and found nothing wrong, but re-run it rather
    // than trust that nothing changed in between -- this is also what actually produces the
    // parsed probs DoAppendMove needs, which UpdateValidation() itself discards.
    const wxString probError = ValidateProbabilities(probs);
    if (!probError.empty()) {
      wxBell();
      RestoreAfterFailedCommit(m_rows.back().probCtrl);
      m_committing = false;
      return false;
    }
  }

  try {
    m_doc->DoAppendMove(m_node, m_player, labels, probs);
  }
  catch (const std::exception &ex) {
    wxTextCtrl *ctrl = m_rows.empty() ? nullptr : m_rows.front().labelCtrl;
    ShowValidationFailure(wxString::FromUTF8(ex.what()), ctrl);
    RestoreAfterFailedCommit(ctrl);
    m_committing = false;
    return false;
  }

  ClearValidationFailure();

  m_dismissing = true;
  Hide();
  m_dismissing = false;
  m_node = nullptr;

  m_committing = false;
  return true;
}

void AppendMovePopup::ShowValidationFailure(const wxString &p_message, wxTextCtrl *p_ctrl)
{
  wxBell();

  if (m_errorText) {
    m_errorText->SetLabel(p_message);
    m_errorText->Wrap(FromDIP(260));
    m_errorText->Show();
  }

  if (m_contentPanel) {
    m_contentPanel->Layout();
  }

  Fit();
  PositionPopup();

  if (p_ctrl) {
    p_ctrl->SetFocus();
    p_ctrl->SelectAll();
  }
}

void AppendMovePopup::ClearValidationFailure()
{
  if (!m_errorText) {
    return;
  }

  m_errorText->SetLabel(wxEmptyString);
  m_errorText->Hide();

  if (m_contentPanel) {
    m_contentPanel->Layout();
  }
}

void AppendMovePopup::RestoreAfterFailedCommit(wxTextCtrl *p_invalidCtrl)
{
  if (m_restoringAfterFailedCommit) {
    return;
  }

  m_restoringAfterFailedCommit = true;

  CallAfter([this, p_invalidCtrl]() {
    m_restoringAfterFailedCommit = false;

    if (!m_node) {
      return;
    }

    PositionPopup();

    if (!IsShown()) {
      Show();
    }

    if (p_invalidCtrl) {
      p_invalidCtrl->SetFocus();
      p_invalidCtrl->SelectAll();
    }
  });
}

//--------------------------------------------------------------------------
//                        class NodeInfoPopup
//--------------------------------------------------------------------------

// A read-only tooltip-like popup showing, for a hovered node, each player's
// expected continuation value from that node, plus -- for a nonterminal node
// -- the mover's realization/belief probabilities and infoset value. Replaces
// the always-visible per-player equilibrium readout that used to live in the
// left-hand player panel.
class NodeInfoPopup : public wxPopupTransientWindow {
public:
  NodeInfoPopup(EfgDisplay *p_owner, const std::shared_ptr<GameDocument> &p_doc);

  void ShowForNode(const GameNode &p_node);

protected:
  void OnDismiss() override { m_node = nullptr; }

private:
  void BuildControls();
  void PositionPopup();

  EfgDisplay *m_owner;
  std::shared_ptr<GameDocument> m_doc;

  GameNode m_node;

  wxPanel *m_contentPanel;

  struct PlayerRow {
    wxStaticText *nameText;
    wxStaticText *valueText;
  };
  std::vector<PlayerRow> m_playerRows;

  wxBoxSizer *m_moveSizer;
  wxStaticText *m_moveHeading;
  wxStaticText *m_nodeProbText;
  wxStaticText *m_infosetValueText;
  wxStaticText *m_infosetProbText;
  wxStaticText *m_beliefText;
};

NodeInfoPopup::NodeInfoPopup(EfgDisplay *p_owner, const std::shared_ptr<GameDocument> &p_doc)
  : wxPopupTransientWindow(p_owner, wxBORDER_NONE), m_owner(p_owner), m_doc(p_doc)
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
  BuildControls();
}

void NodeInfoPopup::BuildControls()
{
  auto *popupSizer = new wxBoxSizer(wxVERTICAL);

  m_contentPanel = new wxPanel(this);
  m_contentPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

  auto *outerSizer = new wxBoxSizer(wxVERTICAL);

  auto *playerGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(12));

  const Game game = m_doc->GetGame();
  for (size_t pl = 1; pl <= game->NumPlayers(); pl++) {
    PlayerRow row;

    row.nameText = new wxStaticText(m_contentPanel, wxID_ANY,
                                    wxString::FromUTF8(game->GetPlayer(pl)->GetLabel()));
    wxFont nameFont = row.nameText->GetFont();
    nameFont.SetWeight(wxFONTWEIGHT_BOLD);
    row.nameText->SetFont(nameFont);
    playerGridSizer->Add(row.nameText, 0, wxALIGN_CENTER_VERTICAL);

    row.valueText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
    playerGridSizer->Add(row.valueText, 0, wxALIGN_CENTER_VERTICAL);

    m_playerRows.push_back(row);
  }

  outerSizer->Add(playerGridSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(12));

  m_moveSizer = new wxBoxSizer(wxVERTICAL);

  m_moveHeading = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
  wxFont headingFont = m_moveHeading->GetFont();
  headingFont.SetWeight(wxFONTWEIGHT_BOLD);
  m_moveHeading->SetFont(headingFont);
  m_moveSizer->Add(m_moveHeading, 0, wxTOP, FromDIP(8));

  m_nodeProbText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
  m_moveSizer->Add(m_nodeProbText, 0, wxTOP, FromDIP(2));
  m_infosetProbText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
  m_moveSizer->Add(m_infosetProbText, 0, wxTOP, FromDIP(2));
  m_beliefText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
  m_moveSizer->Add(m_beliefText, 0, wxTOP, FromDIP(2));
  m_infosetValueText = new wxStaticText(m_contentPanel, wxID_ANY, wxEmptyString);
  m_moveSizer->Add(m_infosetValueText, 0, wxTOP, FromDIP(2));

  outerSizer->Add(m_moveSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(12));

  m_contentPanel->SetSizer(outerSizer);
  popupSizer->Add(m_contentPanel, 1, wxEXPAND | wxALL, FromDIP(1));
  SetSizerAndFit(popupSizer);
}

void NodeInfoPopup::PositionPopup()
{
  auto entry = m_owner->GetLayout().GetNodeEntry(m_node);
  if (!entry) {
    return;
  }

  int clientX, clientY;
  m_owner->CalcScrolledPosition(m_owner->LayoutToDevice(entry->GetX() + 20),
                                m_owner->LayoutToDevice(entry->GetY()), &clientX, &clientY);

  const wxPoint screenPoint = m_owner->ClientToScreen(wxPoint(clientX, clientY));

  Position(screenPoint, wxSize(FromDIP(8), FromDIP(8)));
}

void NodeInfoPopup::ShowForNode(const GameNode &p_node)
{
  m_node = p_node;

  const Game game = m_doc->GetGame();
  const AnalysisOutput &profiles = m_doc->GetWorkspace().GetProfiles();

  for (size_t pl = 1; pl <= m_playerRows.size(); pl++) {
    const GamePlayer player = game->GetPlayer(pl);
    const wxColour color = m_doc->GetStyle().GetPlayerColor(player);
    PlayerRow &row = m_playerRows[pl - 1];

    row.nameText->SetForegroundColour(color);
    row.valueText->SetForegroundColour(color);
    row.valueText->SetLabel(
        wxString::FromUTF8(profiles.GetNodeValue(p_node, static_cast<int>(pl))));
  }

  const bool showMove = !p_node->IsTerminal();
  if (showMove) {
    const GamePlayer mover = p_node->GetPlayer();
    const wxColour moverColor = m_doc->GetStyle().GetPlayerColor(mover);

    m_moveHeading->SetForegroundColour(moverColor);
    m_moveHeading->SetLabel(wxString::FromUTF8(mover->GetLabel()) + _(" to move"));
    m_nodeProbText->SetLabel(_("Pr(Node reached): ") +
                             wxString::FromUTF8(profiles.GetRealizProb(p_node)));
    m_infosetProbText->SetLabel(_("Pr(Infoset reached): ") +
                                wxString::FromUTF8(profiles.GetInfosetProb(p_node)));
    m_beliefText->SetLabel(_("Belief: ") + wxString::FromUTF8(profiles.GetBeliefProb(p_node)));
    m_infosetValueText->SetLabel(_("Payoff | Infoset: ") +
                                 wxString::FromUTF8(profiles.GetInfosetValue(p_node)));
  }
  m_contentPanel->GetSizer()->Show(m_moveSizer, showMove, true);

  m_contentPanel->GetSizer()->Layout();
  Fit();
  PositionPopup();
  Popup();
}

//----------------------------------------------------------------------
//              EfgDisplay: outcome/tooltip popup members
//----------------------------------------------------------------------

void EfgDisplay::InitPopups()
{
  m_outcomeEditor = new OutcomeEditorPopup(this, m_doc);
  m_appendMoveEditor = new AppendMovePopup(this, m_doc);
  m_nodeInfoPopup = new NodeInfoPopup(this, m_doc);
}

void EfgDisplay::PostPendingChanges()
{
  if (m_outcomeEditor->IsShown()) {
    m_outcomeEditor->Commit();
  }
  if (m_appendMoveEditor->IsShown()) {
    m_appendMoveEditor->Commit();
  }
}

void EfgDisplay::BeginEditOutcome(const GameNode &p_node, int p_initialPlayer)
{
  m_outcomeEditor->BeginEdit(p_node, p_initialPlayer);
}

void EfgDisplay::BeginAppendMove(const GameNode &p_node, const GamePlayer &p_player)
{
  m_appendMoveEditor->BeginAppend(p_node, p_player);
}

void EfgDisplay::DismissNodeInfo()
{
  m_hoverTimer.Stop();
  m_hoverNode = nullptr;
  if (m_nodeInfoPopup->IsShown()) {
    m_nodeInfoPopup->Dismiss();
  }
}

void EfgDisplay::OnHoverTimer(wxTimerEvent &)
{
  if (!m_hoverNode || m_doc->GetWorkspace().GetCurrentProfile() <= 0) {
    return;
  }
  m_nodeInfoPopup->ShowForNode(m_hoverNode);
}

void EfgDisplay::OnLeaveWindow(wxMouseEvent &p_event)
{
  DismissNodeInfo();
  p_event.Skip();
}
} // namespace Gambit::GUI
