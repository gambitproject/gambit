//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgtooltip.cc
// Popups for editing a node's outcome and for showing hover information
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

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/popupwin.h>

#include "games.h"

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
  m_nodeInfoPopup = new NodeInfoPopup(this, m_doc);
}

void EfgDisplay::PostPendingChanges()
{
  if (m_outcomeEditor->IsShown()) {
    m_outcomeEditor->Commit();
  }
}

void EfgDisplay::BeginEditOutcome(const GameNode &p_node, int p_initialPlayer)
{
  m_outcomeEditor->BeginEdit(p_node, p_initialPlayer);
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
