//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dleditmove.cc
// Dialog for viewing and editing properties of a move
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
#include <wx/scrolwin.h>
#include <wx/richmsgdlg.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <set>

#include "gambit.h"
#include "dleditmove.h"
#include "valnumber.h"
#include "editlabel.h"

namespace Gambit::GUI {

namespace {
const wxColour kInvalidLabelBg(255, 220, 220);
const wxColour kNewLabelColour(0, 102, 0);      // new action: bold, dark green
const wxColour kRenamedLabelColour(0, 51, 204); // renamed action: italic, strong blue
} // namespace

// An editable, variable-length list of the actions at an information set, colour-coded like
// a source-control diff (added/renamed/deleted).
//
// Each row's *stable label* (its label before this dialog touched it, or a placeholder for a
// newly added row) never changes; it's what `SetActions()` uses to match the row to its
// underlying action regardless of what the user has typed into its *current* label text.
// Deleting an existing row marks it deleted and disables it, with its delete button becoming
// a restore button, rather than removing it outright -- a newly added row has nothing to
// preserve, so deleting one of those does remove it.
class ActionPanel final : public wxScrolledWindow {
public:
  struct Row {
    std::string stableLabel;
    bool isNew = false;     // true for a row added in this dialog, with no prior action at all
    bool isDeleted = false; // true for an existing row marked for deletion (kept, disabled)
    wxString currentLabelText;
    std::unique_ptr<wxString> probValue; // backing store for NumberValidator; heap-stable
                                         // across Row moves within m_rows
    LabelTextCtrl *labelCtrl = nullptr;
    wxTextCtrl *probCtrl = nullptr;
    wxButton *upButton = nullptr;
    wxButton *downButton = nullptr;
    wxButton *deleteButton = nullptr; // doubles as the restore button when isDeleted
  };

private:
  const bool m_isChance;
  std::vector<Row> m_rows;
  wxBoxSizer *m_topSizer;
  wxColour m_defaultBg;
  std::function<void()> m_onChanged;
  // False until the constructor completes, so Rebuild()'s initial call can't notify the
  // owning dialog through its m_actionPanel pointer before that's been assigned.
  bool m_ready = false;
  // True from the moment a row mutation starts until its (deferred) Rebuild() finishes.
  // Destroying/rebuilding row controls can fire a focus-kill event on whatever control had
  // focus, reentrantly, while m_rows or the controls it points to are mid-mutation; handlers
  // that touch either must no-op while this is true rather than act on stale state.
  bool m_rebuilding = false;

  std::string NextPlaceholderLabel() const;
  int ActiveCount() const
  {
    return static_cast<int>(
        std::count_if(m_rows.begin(), m_rows.end(), [](const Row &r) { return !r.isDeleted; }));
  }
  void Rebuild();
  // Colours a (non-deleted) row's label to reflect whether it's new, renamed from its
  // stable label, or unchanged, and sets/clears the "renamed from" tooltip to match.
  static void UpdateRowStyle(Row &row);
  void NotifyChanged() const
  {
    if (m_ready) {
      m_onChanged();
    }
  }

public:
  ActionPanel(wxWindow *p_parent, const GameInfoset &p_infoset,
              const std::function<void()> &p_onChanged);

  int NumActions() const { return static_cast<int>(m_rows.size()); }
  bool IsDeleted(int p_index) const { return m_rows.at(p_index).isDeleted; }
  std::string GetStableLabel(int p_index) const { return m_rows.at(p_index).stableLabel; }
  wxString GetActionLabel(int p_index) const
  {
    return m_rows.at(p_index).labelCtrl->GetNormalizedValue();
  }
  Number GetActionProb(int p_index) const
  {
    return Number(m_rows.at(p_index).probCtrl->GetValue().ToStdString());
  }

  void AddAction();
  // Marks an existing row deleted (or restores an already-deleted one); removes a row added
  // in this dialog outright. A no-op if this would leave no active (non-deleted) rows.
  void ToggleDeleted(int p_index);
  void MoveActionUp(int p_index);
  void MoveActionDown(int p_index);

  // Highlights any empty or duplicate action labels among active (non-deleted) rows, and
  // returns a description of the first problem found, or an empty string if all are valid.
  wxString ValidateLabels();
};

ActionPanel::ActionPanel(wxWindow *p_parent, const GameInfoset &p_infoset,
                         const std::function<void()> &p_onChanged)
  : wxScrolledWindow(p_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxVSCROLL | wxTAB_TRAVERSAL),
    m_isChance(p_infoset->IsChanceInfoset()), m_onChanged(p_onChanged)
{
  for (const auto &action : p_infoset->GetActions()) {
    Row row;
    row.stableLabel = action->GetLabel();
    row.currentLabelText = wxString::FromUTF8(row.stableLabel);
    if (m_isChance) {
      row.probValue = std::make_unique<wxString>(
          static_cast<std::string>(p_infoset->GetActionProb(action)).c_str(), *wxConvCurrent);
    }
    m_rows.push_back(std::move(row));
  }

  m_topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_topSizer);
  SetScrollRate(0, FromDIP(10));

  Rebuild();

  const wxSize bestSize = m_topSizer->CalcMin();
  SetMinSize(
      wxSize(FromDIP(m_isChance ? 460 : 360), std::min(bestSize.GetHeight(), FromDIP(250))));

  if (!m_rows.empty()) {
    m_defaultBg = m_rows.front().labelCtrl->GetBackgroundColour();
  }

  m_ready = true;
}

std::string ActionPanel::NextPlaceholderLabel() const
{
  std::set<std::string> used;
  for (const auto &row : m_rows) {
    used.insert(row.stableLabel);
    used.insert(row.currentLabelText.ToStdString(wxConvUTF8));
  }
  int number = static_cast<int>(m_rows.size()) + 1;
  while (contains(used, std::to_string(number))) {
    number++;
  }
  return std::to_string(number);
}

void ActionPanel::UpdateRowStyle(Row &row)
{
  // Colour alone can be hard to tell apart (or perceive at all); weight/slant give the same
  // information a second way, so the state still reads even if the colours don't.
  wxFont font = row.labelCtrl->GetFont();
  font.SetWeight(wxFONTWEIGHT_NORMAL);
  font.SetStyle(wxFONTSTYLE_NORMAL);

  if (row.isNew) {
    font.SetWeight(wxFONTWEIGHT_BOLD);
    row.labelCtrl->SetForegroundColour(kNewLabelColour);
    row.labelCtrl->UnsetToolTip();
  }
  else if (row.currentLabelText.ToStdString(wxConvUTF8) != row.stableLabel) {
    font.SetStyle(wxFONTSTYLE_ITALIC);
    row.labelCtrl->SetForegroundColour(kRenamedLabelColour);
    row.labelCtrl->SetToolTip(
        wxString::Format(_("Renamed from \"%s\""), wxString::FromUTF8(row.stableLabel)));
  }
  else {
    row.labelCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    row.labelCtrl->UnsetToolTip();
  }
  row.labelCtrl->SetFont(font);
  row.labelCtrl->Refresh();
}

void ActionPanel::Rebuild()
{
  m_rebuilding = true;
  m_topSizer->Clear(true); // destroys the previous row controls; each Row's own state
                           // (stableLabel, currentLabelText, probValue) isn't owned by them

  const int numColumns = m_isChance ? 4 : 3;
  auto *gridSizer = new wxFlexGridSizer(numColumns, 5, 10);
  gridSizer->AddGrowableCol(1, 1);
  if (m_isChance) {
    gridSizer->AddGrowableCol(2, 1);
  }

  gridSizer->AddSpacer(1);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Label")), 0, wxALIGN_CENTER_VERTICAL);
  if (m_isChance) {
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Probability")), 0,
                   wxALIGN_CENTER_VERTICAL);
  }
  gridSizer->AddSpacer(1);

  const int activeCount = ActiveCount();
  const wxSize buttonSize(FromDIP(28), -1);

  for (size_t i = 0; i < m_rows.size(); i++) {
    Row &row = m_rows[i];

    wxString number;
    number << (i + 1);
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, number), 0,
                   wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

    row.labelCtrl = new LabelTextCtrl(this, wxID_ANY, row.currentLabelText);
    if (row.isDeleted) {
      wxFont strikeFont = row.labelCtrl->GetFont();
      strikeFont.SetStrikethrough(true);
      row.labelCtrl->SetFont(strikeFont);
      row.labelCtrl->Disable();
    }
    else {
      UpdateRowStyle(row);
    }
    row.labelCtrl->Bind(wxEVT_TEXT, [this, i](wxCommandEvent &p_event) {
      if (!m_rebuilding && i < m_rows.size()) {
        Row &changedRow = m_rows.at(i);
        changedRow.currentLabelText = changedRow.labelCtrl->GetValue();
        UpdateRowStyle(changedRow);
        NotifyChanged();
      }
      p_event.Skip();
    });
    row.labelCtrl->Bind(wxEVT_KILL_FOCUS, [this, i](wxFocusEvent &p_event) {
      if (!m_rebuilding && i < m_rows.size()) {
        Row &changedRow = m_rows.at(i);
        changedRow.currentLabelText = changedRow.labelCtrl->GetNormalizedValue();
        UpdateRowStyle(changedRow);
        NotifyChanged();
      }
      p_event.Skip();
    });
    gridSizer->Add(row.labelCtrl, 1, wxEXPAND);

    if (m_isChance) {
      row.probCtrl =
          new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0,
                         NumberValidator(row.probValue.get(), Rational(0), Rational(1)));
      row.probCtrl->Enable(!row.isDeleted);
      gridSizer->Add(row.probCtrl, 1, wxEXPAND);
    }

    auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    if (!row.isDeleted) {
      row.upButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2191), wxDefaultPosition, buttonSize);
      row.upButton->Enable(i > 0);
      row.upButton->Bind(wxEVT_BUTTON,
                         [this, i](wxCommandEvent &) { MoveActionUp(static_cast<int>(i)); });
      buttonSizer->Add(row.upButton, 0, wxLEFT, 2);

      row.downButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2193), wxDefaultPosition, buttonSize);
      row.downButton->Enable(i + 1 < m_rows.size());
      row.downButton->Bind(wxEVT_BUTTON,
                           [this, i](wxCommandEvent &) { MoveActionDown(static_cast<int>(i)); });
      buttonSizer->Add(row.downButton, 0, wxLEFT, 2);
    }
    else {
      // Keep the delete/restore button aligned under its counterpart in other rows, in
      // place of the Up/Down buttons a deleted row no longer has. Clear() already
      // destroyed the previous Up/Down controls these pointers referred to; null them out
      // rather than leave them dangling.
      row.upButton = nullptr;
      row.downButton = nullptr;
      buttonSizer->AddSpacer(buttonSize.GetWidth() + 2);
      buttonSizer->AddSpacer(buttonSize.GetWidth() + 2);
    }

    row.deleteButton =
        new wxButton(this, wxID_ANY, row.isDeleted ? wxUniChar(0x21BA) : wxUniChar(0x2715),
                     wxDefaultPosition, buttonSize);
    row.deleteButton->SetToolTip(row.isDeleted ? _("Restore action") : _("Delete action"));
    row.deleteButton->Enable(row.isDeleted || activeCount > 1);
    row.deleteButton->Bind(wxEVT_BUTTON,
                           [this, i](wxCommandEvent &) { ToggleDeleted(static_cast<int>(i)); });
    buttonSizer->Add(row.deleteButton, 0, wxLEFT, 2);

    gridSizer->Add(buttonSizer, 0, wxALIGN_CENTER_VERTICAL);
  }

  // A trailing row with only its button-column cell populated, so the "+" control lands
  // directly below the last row's Up/Down/Delete buttons rather than as a separate control
  // elsewhere in the dialog.
  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  if (m_isChance) {
    gridSizer->AddSpacer(1);
  }
  auto *addButton = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, buttonSize);
  addButton->SetToolTip(_("Add action"));
  addButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { AddAction(); });
  auto *addSizer = new wxBoxSizer(wxHORIZONTAL);
  addSizer->Add(addButton, 0, wxLEFT, 2);
  gridSizer->Add(addSizer, 0, wxALIGN_CENTER_VERTICAL);

  m_topSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);
  FitInside();
  Layout();
  m_rebuilding = false;
  // No NotifyChanged() call here -- see m_ready's comment. Callers that mutate m_rows are
  // responsible for notifying once Rebuild() returns.
}

void ActionPanel::AddAction()
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  Row row;
  row.stableLabel = NextPlaceholderLabel();
  row.isNew = true;
  row.currentLabelText = wxString::FromUTF8(row.stableLabel);
  if (m_isChance) {
    row.probValue = std::make_unique<wxString>("0");
  }
  m_rows.push_back(std::move(row));
  // Rebuild() would destroy the very "+" button whose click is invoking this method, while
  // its handler is still on the call stack -- unsafe in wx. CallAfter() defers it to once
  // this event finishes dispatching; raising m_rebuilding now (not just once Rebuild()
  // starts) covers the gap where m_rows is already mutated but the stale old controls are
  // still alive.
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void ActionPanel::ToggleDeleted(int p_index)
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  Row &row = m_rows.at(p_index);
  if (!row.isDeleted && ActiveCount() <= 1) {
    return; // refuse to drop below one active action
  }
  if (row.isNew) {
    m_rows.erase(m_rows.begin() + p_index);
  }
  else {
    row.isDeleted = !row.isDeleted;
  }
  // See AddAction() for why this defers Rebuild() and raises m_rebuilding immediately.
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void ActionPanel::MoveActionUp(int p_index)
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  if (p_index <= 0) {
    return;
  }
  std::swap(m_rows[p_index - 1], m_rows[p_index]);
  // See AddAction() for why this defers Rebuild() and raises m_rebuilding immediately.
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void ActionPanel::MoveActionDown(int p_index)
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  if (p_index + 1 >= static_cast<int>(m_rows.size())) {
    return;
  }
  std::swap(m_rows[p_index], m_rows[p_index + 1]);
  // See AddAction() for why this defers Rebuild() and raises m_rebuilding immediately.
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

wxString ActionPanel::ValidateLabels()
{
  const int numActions = NumActions();
  wxString message;

  for (int act = 0; act < numActions; act++) {
    if (m_rows[act].isDeleted) {
      continue;
    }
    const wxString value = m_rows[act].labelCtrl->GetValue();
    bool invalid = value.empty();
    for (int other = 0; !invalid && other < numActions; other++) {
      if (other != act && !m_rows[other].isDeleted &&
          m_rows[other].labelCtrl->GetValue() == value) {
        invalid = true;
      }
    }

    m_rows[act].labelCtrl->SetBackgroundColour(invalid ? kInvalidLabelBg : m_defaultBg);
    m_rows[act].labelCtrl->Refresh();

    if (invalid && message.empty()) {
      message = value.empty() ? _("Action labels cannot be empty.")
                              : _("Action labels must be unique within the information set.");
    }
  }
  return message;
}

//======================================================================
//                      class EditMoveDialog
//======================================================================

EditMoveDialog::EditMoveDialog(wxWindow *p_parent, const GameInfoset &p_infoset)
  : wxDialog(p_parent, wxID_ANY, _("Edit move"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_infoset(p_infoset)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Information set label")), 0,
                  wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_infosetLabel = new LabelTextCtrl(this, wxID_ANY, wxString::FromUTF8(p_infoset->GetLabel()));
  m_infosetLabelDefaultBg = m_infosetLabel->GetBackgroundColour();
  m_infosetLabel->Bind(wxEVT_TEXT, [this](wxCommandEvent &p_event) {
    UpdateValidation();
    p_event.Skip();
  });
  m_infosetLabel->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent &p_event) {
    UpdateValidation();
    p_event.Skip();
  });
  labelSizer->Add(m_infosetLabel, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxEXPAND);

  {
    wxString label;
    label << _("Number of members: ") << p_infoset->GetMembers().size();
    topSizer->Add(new wxStaticText(this, wxID_STATIC, label), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
  }

  auto *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, _("Belongs to player")), 0,
                   wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_player = new wxChoice(this, wxID_ANY);
  if (p_infoset->IsChanceInfoset()) {
    m_player->Append(_("Chance"));
    m_player->SetSelection(0);
    m_player->Disable();
  }
  else {
    for (const auto &player : p_infoset->GetGame()->GetPlayers()) {
      wxString label;
      label << player->GetNumber() << ": " << player->GetLabel();
      m_player->Append(label);
    }
    m_player->SetSelection(p_infoset->GetPlayer()->GetNumber() - 1);
  }
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxEXPAND);

  auto *actionBoxSizer =
      new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, _("Actions")), wxVERTICAL);
  // ActionPanel sizes itself to fit its own content (see its constructor); don't override
  // that with a fixed estimate here, which would go stale as soon as the row count changes.
  m_actionPanel = new ActionPanel(this, p_infoset, [this]() { UpdateValidation(); });

  actionBoxSizer->Add(m_actionPanel, 1, wxALL | wxEXPAND, 5);

  topSizer->Add(actionBoxSizer, 1, wxALL | wxEXPAND, 5);

  m_errorText = new wxStaticText(this, wxID_STATIC, wxEmptyString);
  m_errorText->SetForegroundColour(*wxRED);
  topSizer->Add(m_errorText, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

  if (auto *buttons = CreateSeparatedButtonSizer(wxOK | wxCANCEL)) {
    topSizer->Add(buttons, 0, wxALL | wxEXPAND, 5);
  }

  SetSizer(topSizer);
  topSizer->SetSizeHints(this);
  SetSize(GetBestSize());
  SetMinSize(GetSize());
  CenterOnParent();

  Bind(wxEVT_BUTTON, &EditMoveDialog::OnOK, this, wxID_OK);

  UpdateValidation();
}

void EditMoveDialog::UpdateValidation()
{
  wxString message;

  const wxString infosetLabel = m_infosetLabel->GetValue();
  bool infosetValid = true;
  if (!infosetLabel.empty()) {
    for (const auto &infoset : m_infoset->GetPlayer()->GetInfosets()) {
      if (infoset != m_infoset && infoset->GetLabel() == infosetLabel) {
        infosetValid = false;
        break;
      }
    }
  }
  m_infosetLabel->SetBackgroundColour(infosetValid ? m_infosetLabelDefaultBg : kInvalidLabelBg);
  m_infosetLabel->Refresh();
  if (!infosetValid) {
    message = _("Information set label must be unique for the player.");
  }

  const wxString actionMessage = m_actionPanel->ValidateLabels();
  if (message.empty()) {
    message = actionMessage;
  }

  m_errorText->SetLabel(message);
  if (auto *ok = FindWindow(wxID_OK)) {
    ok->Enable(message.empty());
  }
}

void EditMoveDialog::OnOK(wxCommandEvent &p_event)
{
  if (!m_errorText->GetLabel().empty()) {
    return;
  }

  if (!m_infoset->IsChanceInfoset()) {
    p_event.Skip();
    return;
  }
  std::vector<Number> activeProbs;
  for (int i = 0; i < m_actionPanel->NumActions(); i++) {
    if (!m_actionPanel->IsDeleted(i)) {
      activeProbs.push_back(m_actionPanel->GetActionProb(i));
    }
  }
  Array<Number> probs(static_cast<int>(activeProbs.size()));
  for (size_t i = 0; i < activeProbs.size(); i++) {
    probs[static_cast<int>(i) + 1] = activeProbs[i];
  }
  try {
    ValidateDistribution(probs);
  }
  catch (ValueException &) {
    wxRichMessageDialog(this, "Probabilities must be nonnegative numbers summing to one.", "Error",
                        wxOK | wxCENTRE | wxICON_ERROR)
        .ShowModal();
    return;
  }
  p_event.Skip();
}

int EditMoveDialog::NumActions() const { return m_actionPanel->NumActions(); }

bool EditMoveDialog::IsDeleted(int p_index) const { return m_actionPanel->IsDeleted(p_index); }

std::string EditMoveDialog::GetStableLabel(int p_index) const
{
  return m_actionPanel->GetStableLabel(p_index);
}

wxString EditMoveDialog::GetActionLabel(int p_index) const
{
  return m_actionPanel->GetActionLabel(p_index);
}

Number EditMoveDialog::GetActionProb(int p_index) const
{
  return m_actionPanel->GetActionProb(p_index);
}

} // namespace Gambit::GUI
