//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgdisplay.cc
// Implementation of window class to display extensive form tree
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

#include <algorithm> // for std::min
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/dnd.h>
#include <wx/popupwin.h>

#include "gambit.h"

#include "efgdisplay.h"
#include "menuconst.h"
#include "dlexcept.h"
#include "valnumber.h"

namespace Gambit::GUI {

class OutcomeEditorPopup : public wxPopupTransientWindow {
public:
  OutcomeEditorPopup(EfgDisplay *p_owner, GameDocument *p_doc);

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
  GameDocument *m_doc;

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

OutcomeEditorPopup::OutcomeEditorPopup(EfgDisplay *p_owner, GameDocument *p_doc)
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

    payoffSizer->Add(new wxStaticText(m_contentPanel, wxID_ANY,
                                      wxString(gamePlayer->GetLabel().c_str(), *wxConvCurrent)),
                     0, wxALIGN_CENTER_VERTICAL);

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

  m_labelCtrl->SetValue(wxString(outcome->GetLabel().c_str(), *wxConvCurrent));

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
//                       Bitmap drawing functions
//--------------------------------------------------------------------------

static wxBitmap MakeOutcomeBitmap()
{
  wxBitmap bitmap(24, 24);
  wxMemoryDC dc;
  dc.SelectObject(bitmap);
  dc.Clear();
  dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));
  // Make a gold-colored background
  dc.SetBrush(wxBrush(wxColour(255, 215, 0), wxBRUSHSTYLE_SOLID));
  dc.DrawCircle(12, 12, 10);
  dc.SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  dc.SetTextForeground(wxColour(0, 192, 0));

  int width, height;
  dc.GetTextExtent(wxT("u"), &width, &height);
  dc.DrawText(wxT("u"), 12 - width / 2, 12 - height / 2);
  return bitmap;
}

//--------------------------------------------------------------------------
//                      class PlayerDropTarget
//--------------------------------------------------------------------------

class PlayerDropTarget : public wxTextDropTarget {
  EfgDisplay *m_owner;
  GameDocument *m_model;

  bool OnDropPlayer(const GameNode &p_node, const wxString &p_text, const wxPoint &p_pos);
  bool OnDropOutcome(const GameNode &p_node, const wxString &p_text, const wxPoint &p_pos);
  bool OnDropTreeNode(const GameNode &p_node, const wxString &p_text, const wxPoint &p_pos);

public:
  explicit PlayerDropTarget(EfgDisplay *p_owner)
    : m_owner(p_owner), m_model(p_owner->GetDocument())
  {
  }

  bool OnDropText(wxCoord x, wxCoord y, const wxString &p_text) override;
};

//
// This recurses the subtree starting at 'p_node' looking for a node
// with the ID 'p_id'.
//
static GameNode GetNode(const GameNode &p_node, int p_id)
{
  if (p_node->GetNumber() == p_id) {
    return p_node;
  }
  if (p_node->IsTerminal()) {
    return nullptr;
  }
  for (const auto &child : p_node->GetChildren()) {
    if (const auto node = GetNode(child, p_id)) {
      return node;
    }
  }
  return nullptr;
}

bool PlayerDropTarget::OnDropPlayer(const GameNode &p_node, const wxString &p_text,
                                    const wxPoint &p_pos)
{
  long pl;
  if (!p_text.Right(p_text.Length() - 1).ToLong(&pl)) {
    return false;
  }

  const Game efg = m_model->GetGame();
  const GamePlayer player = ((pl == 0) ? efg->GetChance() : efg->GetPlayer(pl));

  return m_owner->ShowPlayerDropMenu(p_node, player, p_pos);
}

bool PlayerDropTarget::OnDropOutcome(const GameNode &p_node, const wxString &p_text,
                                     const wxPoint &p_pos)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);

  const GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode || srcNode == p_node || !srcNode->GetOutcome()) {
    return false;
  }

  return m_owner->ShowOutcomeDropMenu(p_node, srcNode, p_pos);
}

bool PlayerDropTarget::OnDropTreeNode(const GameNode &p_node, const wxString &p_text,
                                      const wxPoint &p_pos)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);

  const GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode || srcNode == p_node || srcNode->IsTerminal()) {
    return false;
  }

  return m_owner->ShowTreeDropMenu(p_node, srcNode, p_pos);
}

bool PlayerDropTarget::OnDropText(wxCoord p_x, wxCoord p_y, const wxString &p_text)
{
  const Game efg = m_owner->GetDocument()->GetGame();

  int x, y;
  m_owner->CalcUnscrolledPosition(p_x, p_y, &x, &y);

  x = m_owner->DeviceToLayout(x);
  y = m_owner->DeviceToLayout(y);

  const GameNode node = m_owner->GetLayout().NodeHitTest(x, y);
  if (!node) {
    return false;
  }

  try {
    switch (static_cast<char>(p_text[0])) {
    case 'N':
      return OnDropTreeNode(node, p_text, wxPoint(p_x, p_y));
    case 'P':
      return OnDropPlayer(node, p_text, wxPoint(p_x, p_y));
    case 'O':
      return OnDropOutcome(node, p_text, wxPoint(p_x, p_y));
    default:
      return false;
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(m_owner, ex.what()).ShowModal();
    return false;
  }
}

//----------------------------------------------------------------------
//                      EfgDisplay: Member functions
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(EfgDisplay, wxScrolledWindow)
EVT_MOTION(EfgDisplay::OnMouseMotion)
EVT_LEFT_DOWN(EfgDisplay::OnLeftClick)
EVT_LEFT_DCLICK(EfgDisplay::OnLeftDoubleClick)
EVT_MAGNIFY(EfgDisplay::OnMagnify)
EVT_RIGHT_DOWN(EfgDisplay::OnRightClick)
EVT_KEY_DOWN(EfgDisplay::OnKeyEvent)
EVT_SIZE(EfgDisplay::OnSize)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//                EfgDisplay: Constructor and destructor
//----------------------------------------------------------------------

EfgDisplay::EfgDisplay(wxWindow *p_parent, GameDocument *p_doc)
  : wxScrolledWindow(p_parent), GameView(p_doc), m_layout(p_doc), m_zoom(100),
    m_outcomeEditor(new OutcomeEditorPopup(this, p_doc))
{
  wxWindow::SetBackgroundColour(wxColour(250, 250, 250));

  wxWindow::SetDropTarget(new PlayerDropTarget(this));
  MakeMenus();
  OnUpdate();
}

void EfgDisplay::MakeMenus()
{
  m_nodeMenu = new wxMenu;
  m_nodeMenu->Append(GBT_MENU_EDIT_INSERT_MOVE, _("&Insert move"), _("Insert a move"));
  m_nodeMenu->Append(GBT_MENU_EDIT_INSERT_ACTION, _("Insert &action"),
                     _("Insert an action at the current move"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REVEAL, _("&Reveal"), _("Reveal choice at node"));
  m_nodeMenu->AppendSeparator();

  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_TREE, _("&Delete subtree"),
                     _("Delete the subtree starting at the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_PARENT, _("Delete &parent"),
                     _("Delete the node directly before the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REMOVE_OUTCOME, _("Remove &outcome"),
                     _("Remove the outcome from the selected node"));
  m_nodeMenu->AppendSeparator();

  m_nodeMenu->Append(GBT_MENU_EDIT_NODE, _("&Node properties"), _("Edit properties of the node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_MOVE, _("&Move properties"), _("Edit properties of the move"));

  m_nodeMenu->AppendSeparator();
  m_nodeMenu->Append(GBT_MENU_EDIT_GAME, _("&Game properties"), _("Edit properties of the game"));
}

bool EfgDisplay::ShowPlayerDropMenu(const GameNode &p_targetNode, const GamePlayer &p_player,
                                    const wxPoint &p_pos)
{
  if (!p_targetNode || !p_player) {
    return false;
  }

  const int operationId = wxWindow::NewControlId();

  wxMenu menu;

  if (p_targetNode->IsTerminal()) {
    menu.Append(operationId, _("Insert move for this player"));
  }
  else if (p_targetNode->GetPlayer() == p_player) {
    menu.Append(operationId, _("Insert action at this move"));
  }
  else {
    menu.Append(operationId, _("Assign this move to this player"));
  }

  const int selection = GetPopupMenuSelectionFromUser(menu, p_pos);
  if (selection != operationId) {
    return false;
  }

  try {
    if (p_targetNode->IsTerminal()) {
      m_doc->DoInsertMove(p_targetNode, p_player, 2);
    }
    else if (p_targetNode->GetPlayer() == p_player) {
      m_doc->DoInsertAction(p_targetNode);
    }
    else {
      m_doc->DoSetPlayer(p_targetNode, p_player);
    }

    return true;
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }

  return false;
}

bool EfgDisplay::ShowTreeDropMenu(const GameNode &p_targetNode, const GameNode &p_sourceNode,
                                  const wxPoint &p_pos)
{
  if (!p_targetNode || !p_sourceNode || p_sourceNode->IsTerminal()) {
    return false;
  }

  const bool canCopyOrMoveTree = p_targetNode->IsTerminal();
  const bool canUseSameInfoset =
      (!p_targetNode->IsTerminal() &&
       p_targetNode->GetChildren().size() == p_sourceNode->GetChildren().size()) ||
      p_targetNode->IsTerminal();

  if (!canCopyOrMoveTree && !canUseSameInfoset) {
    return false;
  }

  const int copyTreeId = wxWindow::NewControlId();
  const int moveTreeId = wxWindow::NewControlId();
  const int infosetId = wxWindow::NewControlId();

  wxMenu menu;

  if (canCopyOrMoveTree) {
    menu.Append(copyTreeId, _("Copy subtree here"));
    menu.Append(moveTreeId, _("Move subtree here"));
  }

  if (canUseSameInfoset) {
    if (!menu.GetMenuItems().empty()) {
      menu.AppendSeparator();
    }

    if (p_targetNode->IsTerminal()) {
      menu.Append(infosetId, _("Insert move using same information set"));
    }
    else {
      menu.Append(infosetId, _("Put node in same information set"));
    }
  }

  const int selection = GetPopupMenuSelectionFromUser(menu, p_pos);

  try {
    if (selection == copyTreeId) {
      m_doc->DoCopyTree(p_targetNode, p_sourceNode);
      return true;
    }
    if (selection == moveTreeId) {
      m_doc->DoMoveTree(p_targetNode, p_sourceNode);
      return true;
    }
    if (selection == infosetId) {
      if (!p_targetNode->IsTerminal()) {
        m_doc->DoSetInfoset(p_targetNode, p_sourceNode->GetInfoset());
      }
      else {
        m_doc->DoAppendMove(p_targetNode, p_sourceNode->GetInfoset());
      }
      return true;
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }

  return false;
}

bool EfgDisplay::ShowOutcomeDropMenu(const GameNode &p_targetNode, const GameNode &p_sourceNode,
                                     const wxPoint &p_pos)
{
  if (!p_targetNode || !p_sourceNode || p_targetNode == p_sourceNode ||
      !p_sourceNode->GetOutcome()) {
    return false;
  }

  const int useSameOutcomeId = wxWindow::NewControlId();
  const int copyOutcomeId = wxWindow::NewControlId();
  const int moveOutcomeId = wxWindow::NewControlId();

  wxMenu menu;
  menu.Append(useSameOutcomeId, _("Use same outcome here"));
  menu.Append(copyOutcomeId, _("Copy outcome here"));
  menu.AppendSeparator();
  menu.Append(moveOutcomeId, _("Move outcome here"));

  const int selection = GetPopupMenuSelectionFromUser(menu, p_pos);

  try {
    if (selection == useSameOutcomeId) {
      m_doc->DoSetOutcome(p_targetNode, p_sourceNode->GetOutcome());
      return true;
    }
    if (selection == copyOutcomeId) {
      m_doc->DoCopyOutcome(p_targetNode, p_sourceNode->GetOutcome());
      return true;
    }
    if (selection == moveOutcomeId) {
      m_doc->DoSetOutcome(p_targetNode, p_sourceNode->GetOutcome());
      m_doc->DoSetOutcome(p_sourceNode, nullptr);
      return true;
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }

  return false;
}

//---------------------------------------------------------------------
//                  EfgDisplay: Event-hook members
//---------------------------------------------------------------------

namespace {
GameNode PriorSameInfoset(const GameNode &n)
{
  const GameInfoset infoset = n->GetInfoset();
  if (!infoset) {
    return nullptr;
  }
  const auto members = infoset->GetMembers();
  if (auto node = std::find(members.begin(), members.end(), n); node != members.begin()) {
    return *std::prev(node);
  }
  return nullptr;
}

GameNode NextSameInfoset(const GameNode &n)
{
  const GameInfoset infoset = n->GetInfoset();
  if (!infoset) {
    return nullptr;
  }
  const auto members = infoset->GetMembers();
  if (auto node = std::find(members.begin(), members.end(), n);
      node != members.end() && std::next(node) != members.end()) {
    return *std::next(node);
  }
  return nullptr;
}
} // namespace

void EfgDisplay::OnSize(wxSizeEvent &p_event)
{
  if (m_pendingInitialZoom) {
    const wxSize size = p_event.GetSize();
    if (size.GetWidth() > 50 && size.GetHeight() > 50) {
      FitZoom();
      m_pendingInitialZoom = false;
      FocusNode(m_doc->GetGame()->GetRoot(), 0.18, 0.5);
    }
  }

  p_event.Skip();
}

//
// OnKeyEvent -- handle keypress events.
//
// Navigation shortcuts:
//     left arrow:   go to rendered ancestor of selected node
//     right arrow:  go to rendered descendant of selected node
//     up arrow:     go to previous node at the same rendered level
//     down arrow:   go to next node at the same rendered level
//     ALT-up:       go to previous member of information set
//     ALT-down:     go to next member of information set
//     space:        ensure the selected node is visible
//     home:         select the root node and make it visible
//
// Editing shortcuts:
//     'M', 'm':     edit the move at the selected node
//     return/enter: edit the properties of the selected node
//
// Payoff edit mode only:
//     escape:       cancel edit of payoff
//     tab:          accept edit of payoff, edit next payoff if any
//
void EfgDisplay::OnKeyEvent(wxKeyEvent &p_event)
{
  if (p_event.GetKeyCode() == WXK_HOME) {
    m_doc->SetSelectNode(m_doc->GetGame()->GetRoot());
    FocusNode(m_doc->GetSelectNode());
    return;
  }

  // After this point, all events involve moving relative to selected node.
  // So if there isn't a selected node, the event doesn't apply
  const GameNode selectNode = m_doc->GetSelectNode();
  if (!selectNode) {
    p_event.Skip();
    return;
  }

  switch (p_event.GetKeyCode()) {
  case 'M':
  case 'm': {
    const wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_MOVE);
    wxPostEvent(this, event);
    return;
  }
  case WXK_RETURN:
  case WXK_NUMPAD_ENTER: {
    const wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_NODE);
    wxPostEvent(this, event);
    return;
  }
  case WXK_LEFT:
    if (selectNode->GetParent()) {
      m_doc->SetSelectNode(m_layout.GetRenderedAncestor(selectNode)->GetNode());
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  case WXK_RIGHT:
    if (m_layout.GetRenderedDescendant(selectNode)) {
      m_doc->SetSelectNode(m_layout.GetRenderedDescendant(selectNode)->GetNode());
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  case WXK_UP: {
    const GameNode prior = ((!p_event.AltDown()) ? m_layout.PriorSameLevel(selectNode)
                                                 : PriorSameInfoset(selectNode));
    if (prior) {
      m_doc->SetSelectNode(prior);
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  }
  case WXK_DOWN: {
    const GameNode next =
        ((!p_event.AltDown()) ? m_layout.NextSameLevel(selectNode) : NextSameInfoset(selectNode));
    if (next) {
      m_doc->SetSelectNode(next);
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  }
  case WXK_SPACE:
    EnsureNodeVisible(m_doc->GetSelectNode());
    return;
  default:
    // If nothing else applies, let event propagate
    p_event.Skip();
  }
}

//---------------------------------------------------------------------
//           EfgDisplay: Implementing GameView members
//---------------------------------------------------------------------

void EfgDisplay::PostPendingChanges()
{
  if (m_outcomeEditor->IsShown()) {
    m_outcomeEditor->Commit();
  }
}

void EfgDisplay::OnUpdate()
{
  // First make sure that the selected node is in fact still valid
  if (m_doc->GetSelectNode()) {
    auto entry = m_layout.GetNodeEntry(m_doc->GetSelectNode());
    if (!entry) {
      m_doc->SetSelectNode(nullptr);
    }
  }

  // Force a rebuild on every change for now.
  RefreshTree();

  const GameNode selectNode = m_doc->GetSelectNode();

  m_nodeMenu->Enable(GBT_MENU_EDIT_INSERT_MOVE, selectNode);
  m_nodeMenu->Enable(GBT_MENU_EDIT_INSERT_ACTION, selectNode && selectNode->GetInfoset());
  m_nodeMenu->Enable(GBT_MENU_EDIT_REVEAL, selectNode && selectNode->GetInfoset());
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_TREE, selectNode && !selectNode->IsTerminal());
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_PARENT, selectNode && selectNode->GetParent());
  m_nodeMenu->Enable(GBT_MENU_EDIT_REMOVE_OUTCOME, selectNode && selectNode->GetOutcome());
  m_nodeMenu->Enable(GBT_MENU_EDIT_NODE, selectNode);
  m_nodeMenu->Enable(GBT_MENU_EDIT_MOVE, selectNode && selectNode->GetInfoset());
}

//---------------------------------------------------------------------
//                   EfgDisplay: Drawing functions
//---------------------------------------------------------------------

void EfgDisplay::RefreshTree()
{
  m_layout.Layout(m_doc->GetGame());
  AdjustScrollbarSteps();
  Refresh();
}

constexpr int kScrollPixelsPerUnit = 1;

void EfgDisplay::AdjustScrollbarSteps()
{
  int oldPixelsPerUnitX, oldPixelsPerUnitY;
  GetScrollPixelsPerUnit(&oldPixelsPerUnitX, &oldPixelsPerUnitY);

  int scrollX, scrollY;
  GetViewStart(&scrollX, &scrollY);

  const int currentPixelX = scrollX * oldPixelsPerUnitX;
  const int currentPixelY = scrollY * oldPixelsPerUnitY;

  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  const int virtualWidth = LayoutToDevice(m_layout.MaxX());
  const int virtualHeight = LayoutToDevice(m_layout.MaxY());

  const int maxPixelX = std::max(0, virtualWidth - clientWidth);
  const int maxPixelY = std::max(0, virtualHeight - clientHeight);

  const int clampedPixelX = std::clamp(currentPixelX, 0, maxPixelX);
  const int clampedPixelY = std::clamp(currentPixelY, 0, maxPixelY);

  SetScrollbars(kScrollPixelsPerUnit, kScrollPixelsPerUnit,
                virtualWidth / kScrollPixelsPerUnit + 1, virtualHeight / kScrollPixelsPerUnit + 1,
                clampedPixelX / kScrollPixelsPerUnit, clampedPixelY / kScrollPixelsPerUnit);
}

void EfgDisplay::FitZoom()
{
  int width, height;
  GetClientSize(&width, &height);

  double zoomx = static_cast<double>(width) / static_cast<double>(m_layout.MaxX());
  double zoomy = static_cast<double>(height) / static_cast<double>(m_layout.MaxY());

  zoomx = std::min(zoomx, 1.0);
  zoomy = std::min(zoomy, 1.0); // never zoom in (only out)
  const int fittedZoom = static_cast<int>(100.0 * (std::min(zoomx, zoomy) * .9));
  m_zoom = std::max(50, fittedZoom);
  AdjustScrollbarSteps();
  Refresh();
}

namespace {

constexpr int kMinZoom = 10;
constexpr int kMaxZoom = 150;
constexpr int kZoomStep = 10;
constexpr int kScrollPixelsPerUnit = 1;

int ClampZoom(int p_zoom) { return std::clamp(p_zoom, kMinZoom, kMaxZoom); }

} // namespace

void EfgDisplay::SetZoom(int p_zoom, bool p_keepSelectionVisible)
{
  const int zoom = ClampZoom(p_zoom);
  if (zoom == m_zoom) {
    return;
  }

  m_zoom = zoom;
  AdjustScrollbarSteps();

  if (p_keepSelectionVisible) {
    EnsureNodeVisible(m_doc->GetSelectNode());
  }

  Refresh();
}

void EfgDisplay::ZoomByFactor(double p_factor, const wxPoint &p_clientPoint)
{
  if (p_factor <= 0.0) {
    return;
  }

  const int oldZoom = GetZoom();
  const int newZoom = ClampZoom(static_cast<int>(std::lround(oldZoom * p_factor)));

  if (newZoom == oldZoom) {
    return;
  }

  int unscrolledX, unscrolledY;
  CalcUnscrolledPosition(p_clientPoint.x, p_clientPoint.y, &unscrolledX, &unscrolledY);

  const double oldScale = GetZoom() / 100.0;
  const double layoutX = unscrolledX / oldScale;
  const double layoutY = unscrolledY / oldScale;

  SetZoom(newZoom, false);

  const double newScale = GetZoom() / 100.0;
  const int targetUnscrolledX = static_cast<int>(std::lround(layoutX * newScale));
  const int targetUnscrolledY = static_cast<int>(std::lround(layoutY * newScale));

  int pixelsPerUnitX, pixelsPerUnitY;
  GetScrollPixelsPerUnit(&pixelsPerUnitX, &pixelsPerUnitY);

  if (pixelsPerUnitX <= 0 || pixelsPerUnitY <= 0) {
    return;
  }

  const int targetScrollX = targetUnscrolledX - p_clientPoint.x;
  const int targetScrollY = targetUnscrolledY - p_clientPoint.y;

  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  int virtualWidth, virtualHeight;
  GetVirtualSize(&virtualWidth, &virtualHeight);

  const int maxPixelX = std::max(0, virtualWidth - clientWidth);
  const int maxPixelY = std::max(0, virtualHeight - clientHeight);

  const int clampedScrollX = std::clamp(targetScrollX, 0, maxPixelX);
  const int clampedScrollY = std::clamp(targetScrollY, 0, maxPixelY);

  Scroll(clampedScrollX / pixelsPerUnitX, clampedScrollY / pixelsPerUnitY);
}

void EfgDisplay::OnDraw(wxDC &p_dc)
{
  p_dc.SetUserScale(GetScale(), GetScale());
  p_dc.Clear();
  const int maxX = m_layout.MaxX();
  m_layout.Render(p_dc, false);
  // When we draw, we might change the location of the right margin
  // (because of the outcome labels).  Make sure scrollbars are adjusted
  // to reflect this.
  if (maxX != m_layout.MaxX()) {
    AdjustScrollbarSteps();
  }
}

void EfgDisplay::OnDraw(wxDC &p_dc, double p_zoom)
{
  // A bit of a hack: this allows us to set zoom separately in printout code
  const int saveZoom = m_zoom;
  m_zoom = static_cast<int>(100.0 * p_zoom);

  p_dc.SetUserScale(GetScale(), GetScale());
  p_dc.Clear();
  const int maxX = m_layout.MaxX();
  // A second hack: this is usually only called by functions for hardcopy
  // output (printouts or graphics images).  We want to suppress the
  // use of the "hints" for these.
  // FIXME: Of course, this hack implies some useful refactor is called for!
  m_layout.Render(p_dc, true);
  // When we draw, we might change the location of the right margin
  // (because of the outcome labels).  Make sure scrollbars are adjusted
  // to reflect this.
  if (maxX != m_layout.MaxX()) {
    AdjustScrollbarSteps();
  }

  m_zoom = saveZoom;
}

void EfgDisplay::FocusNode(const GameNode &p_node, double p_xFrac, double p_yFrac)
{
  if (!p_node) {
    return;
  }

  auto entry = m_layout.GetNodeEntry(p_node);
  if (!entry) {
    return;
  }

  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  const int targetX = LayoutToDevice(entry->GetX()) - clientWidth * p_xFrac;
  const int targetY = LayoutToDevice(entry->GetY()) - clientHeight * p_yFrac;

  int pixelsPerUnitX, pixelsPerUnitY;
  GetScrollPixelsPerUnit(&pixelsPerUnitX, &pixelsPerUnitY);

  int virtualWidth, virtualHeight;
  GetVirtualSize(&virtualWidth, &virtualHeight);

  const int maxPixelX = std::max(0, virtualWidth - clientWidth);
  const int maxPixelY = std::max(0, virtualHeight - clientHeight);

  const int clampedX = std::clamp(targetX, 0, maxPixelX);
  const int clampedY = std::clamp(targetY, 0, maxPixelY);

  Scroll(clampedX / pixelsPerUnitX, clampedY / pixelsPerUnitY);
}

void EfgDisplay::EnsureNodeVisible(const GameNode &p_node)
{
  if (!p_node) {
    return;
  }

  int pixelsPerUnitX, pixelsPerUnitY;
  GetScrollPixelsPerUnit(&pixelsPerUnitX, &pixelsPerUnitY);

  auto entry = m_layout.GetNodeEntry(p_node);
  int xScroll, yScroll;
  GetViewStart(&xScroll, &yScroll);
  int width, height;
  GetClientSize(&width, &height);

  int xx, yy;
  CalcScrolledPosition(LayoutToDevice(entry->GetX()) - 20, LayoutToDevice(entry->GetY()), &xx,
                       &yy);
  if (xx < 0) {
    xScroll -= -xx / pixelsPerUnitX + 1;
  }

  CalcScrolledPosition(LayoutToDevice(entry->GetX()), LayoutToDevice(entry->GetY()), &xx, &yy);
  if (xx > width) {
    xScroll += (xx - width) / pixelsPerUnitX + 1;
  }
  if (xScroll < 0) {
    xScroll = 0;
  }
  else if (xScroll > GetScrollRange(wxHORIZONTAL)) {
    xScroll = GetScrollRange(wxHORIZONTAL);
  }

  CalcScrolledPosition(LayoutToDevice(entry->GetX()), LayoutToDevice(entry->GetY()) - 20, &xx,
                       &yy);
  if (yy < 0) {
    yScroll -= -yy / pixelsPerUnitY + 1;
  }
  CalcScrolledPosition(LayoutToDevice(entry->GetX()), LayoutToDevice(entry->GetY()) + 20, &xx,
                       &yy);
  if (yy > height) {
    yScroll += (yy - height) / pixelsPerUnitY + 1;
  }
  if (yScroll < 0) {
    yScroll = 0;
  }
  else if (yScroll > GetScrollRange(wxVERTICAL)) {
    yScroll = GetScrollRange(wxVERTICAL);
  }

  Scroll(xScroll, yScroll);
}

//
// Left mouse button click:
// Without key modifiers, selects a node
// With shift key, selects whole subtree (not yet implemented)
// With control key, adds node to selection (not yet implemented)
//
void EfgDisplay::OnLeftClick(wxMouseEvent &p_event)
{
  SetFocus();

  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = DeviceToLayout(x);
  y = DeviceToLayout(y);

  const GameNode node = m_layout.NodeHitTest(x, y);
  if (node != m_doc->GetSelectNode()) {
    m_doc->SetSelectNode(node);
  }
}

//
// Left mouse button double-click:
// Sets selection, brings up node properties dialog
//
void EfgDisplay::OnLeftDoubleClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = DeviceToLayout(x);
  y = DeviceToLayout(y);

  GameNode node = m_layout.NodeHitTest(x, y);
  if (node) {
    m_doc->SetSelectNode(node);
    const wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_NODE);
    wxPostEvent(this, event);
    return;
  }

  node = m_layout.OutcomeHitTest(x, y);
  if (node) {
    int initialPlayer = 0;

    if (node->GetOutcome()) {
      auto entry = m_layout.GetNodeEntry(node);

      for (size_t player = 1; player <= m_doc->GetGame()->NumPlayers(); ++player) {
        if (entry->GetPayoffExtent(player).Contains(x, y)) {
          initialPlayer = static_cast<int>(player);
          break;
        }
      }
    }

    m_outcomeEditor->BeginEdit(node, initialPlayer);
    return;
  }

  if (m_doc->GetStyle().GetBranchBelowLabel() == GBT_BRANCH_LABEL_LABEL) {
    node = m_layout.BranchBelowHitTest(x, y);
    if (node) {
      m_doc->SetSelectNode(node);
      const wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_MOVE);
      wxPostEvent(this, event);
      return;
    }
  }
}

void EfgDisplay::OnMagnify(wxMouseEvent &p_event)
{
  if (const double factor = 1.0 + p_event.GetMagnification(); factor > 0.0) {
    ZoomByFactor(factor, p_event.GetPosition());
  }
}

void EfgDisplay::OnMouseMotion(wxMouseEvent &p_event)
{
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    int x, y;
    CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
    x = DeviceToLayout(x);
    y = DeviceToLayout(y);

    GameNode node = m_layout.NodeHitTest(x, y);

    if (node && !node->IsTerminal()) {
      wxString label;
      label << "N" << node->GetNumber();
      wxTextDataObject textData(label);

      wxDropSource source(textData, this);
      source.DoDragDrop(wxDrag_DefaultMove);
      return;
    }

    node = m_layout.OutcomeHitTest(x, y);

    if (node && node->GetOutcome()) {
      wxString label;
      label << "O" << node->GetNumber();
      wxTextDataObject textData(label);

      wxDropSource source(textData, this);
      source.DoDragDrop(wxDrag_DefaultMove);
    }
  }
}

//
// Right mouse-button click:
// Set selection, display context-sensitive popup menu
//
void EfgDisplay::OnRightClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = DeviceToLayout(x);
  y = DeviceToLayout(y);

  const GameNode node = m_layout.NodeHitTest(x, y);
  if (node != m_doc->GetSelectNode()) {
    m_doc->SetSelectNode(node);
  }
  PopupMenu(m_nodeMenu);
}
} // namespace Gambit::GUI
