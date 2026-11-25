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
#include <wx/bmpcbox.h>
#include <wx/statline.h>

#include <algorithm>
#include <functional>
#include <memory>

#include "games.h"
#include "games/gametree.h"
#include "gamedoc.h"
#include "dleditmove.h"
#include "dialogstyle.h"
#include "valnumber.h"
#include "editlabel.h"
#include "labellist.h"

namespace Gambit::GUI {

// An editable, variable-length list of the actions at an infoset, colour-coded like
// a source-control diff (added/renamed/deleted), with a probability per action at a chance
// move. The rows are held in a `LabelListEdit`; this panel draws them and forwards the user's
// actions to it.
class ActionPanel final : public wxScrolledWindow {
  // Each row's probability text, the backing store for its NumberValidator; heap-stable across
  // row moves. Null for a personal move.
  using ProbValue = std::unique_ptr<wxString>;

  const bool m_isChance;
  LabelListEdit<ProbValue> m_list;
  // One of each per row of m_list, recreated by each Rebuild(); m_probCtrls only at chance
  std::vector<LabelTextCtrl *> m_labelCtrls;
  std::vector<wxTextCtrl *> m_probCtrls;
  wxBoxSizer *m_topSizer;
  wxColour m_defaultBg;
  std::function<void()> m_onChanged;
  // False until the constructor completes, so Rebuild()'s initial call can't notify the
  // owning dialog through its m_actionPanel pointer before that's been assigned.
  bool m_ready = false;
  // True from the moment a row mutation starts until its (deferred) Rebuild() finishes; see
  // StrategyPanel (dleditstrategies.cc) for why this guard is needed.
  bool m_rebuilding = false;

  void Rebuild();
  // See StrategyPanel::ScheduleRebuild() (dleditstrategies.cc).
  void ScheduleRebuild(bool p_scrollToBottom = false);
  // Scrolls to show the last row -- called after AddAction() so the row just added (which a
  // long list would otherwise leave below the fold) is immediately visible.
  void ScrollToBottom();
  void NotifyChanged() const
  {
    if (m_ready) {
      m_onChanged();
    }
  }

public:
  ActionPanel(wxWindow *p_parent, const GameInfoset &p_infoset,
              const std::function<void()> &p_onChanged);

  // The rows as edited, with each label as normalized in its control
  const LabelListEdit<ProbValue> &GetEdit();
  // The probabilities currently typed into the active rows, in order (chance moves only)
  std::vector<Number> GetActiveProbs() const;

  void AddAction();
  // Marks an existing row deleted (or restores an already-deleted one); removes a row added
  // in this dialog outright. A no-op if this would leave no active (non-deleted) rows.
  void ToggleDeleted(int p_index);
  void MoveActionUp(int p_index);
  void MoveActionDown(int p_index);

  // Highlights any empty or duplicate action labels among active (non-deleted) rows, and
  // returns a description of the first problem found, or an empty string if all are valid.
  wxString CheckLabels();
  // At a chance move, highlights the probabilities of active rows if any is not a number or
  // they do not form a distribution, and returns a description of the problem, or an empty
  // string if there is none.
  wxString CheckProbabilities();
};

ActionPanel::ActionPanel(wxWindow *p_parent, const GameInfoset &p_infoset,
                         const std::function<void()> &p_onChanged)
  : wxScrolledWindow(p_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxVSCROLL | wxTAB_TRAVERSAL),
    m_isChance(p_infoset->IsChanceInfoset()), m_topSizer(new wxBoxSizer(wxVERTICAL)),
    m_onChanged(p_onChanged)
{
  // Explicit, rather than inherited from the dialog: this is what gives the action list a
  // visible contrast against the shaded header panel above it, the same white-against-shaded
  // distinction AppendMovePopup uses.
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

  for (const auto &action : p_infoset->GetActions()) {
    ProbValue probValue;
    if (m_isChance) {
      probValue = std::make_unique<wxString>(
          static_cast<std::string>(p_infoset->GetActionProb(action)).c_str(), *wxConvCurrent);
    }
    m_list.AddExisting(action->GetLabel(), std::move(probValue));
  }

  SetSizer(m_topSizer);
  SetScrollRate(0, FromDIP(10));

  Rebuild();

  const wxSize bestSize = m_topSizer->CalcMin();
  SetMinSize(
      wxSize(FromDIP(m_isChance ? 460 : 360), std::min(bestSize.GetHeight(), FromDIP(250))));

  if (!m_labelCtrls.empty()) {
    m_defaultBg = m_labelCtrls.front()->GetBackgroundColour();
  }

  m_ready = true;
}

const LabelListEdit<ActionPanel::ProbValue> &ActionPanel::GetEdit()
{
  for (size_t i = 0; i < m_labelCtrls.size(); i++) {
    m_list.SetLabel(i, m_labelCtrls[i]->GetNormalizedValue().ToStdString(wxConvUTF8));
  }
  return m_list;
}

std::vector<Number> ActionPanel::GetActiveProbs() const
{
  std::vector<Number> probs;
  for (size_t i = 0; i < m_probCtrls.size(); i++) {
    if (!m_list.GetRow(i).isDeleted) {
      probs.emplace_back(m_probCtrls[i]->GetValue().ToStdString());
    }
  }
  return probs;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void ActionPanel::Rebuild()
{
  m_rebuilding = true;
  m_topSizer->Clear(true); // destroys the previous row controls; the rows themselves
                           // (including the probability text) are in m_list
  m_labelCtrls.clear();
  m_probCtrls.clear();

  const int numColumns = m_isChance ? 3 : 2;
  auto *gridSizer =
      new wxFlexGridSizer(numColumns, FromDIP(c_spaceTight), FromDIP(c_spaceRelated));
  gridSizer->AddGrowableCol(0, 1);
  if (m_isChance) {
    gridSizer->AddGrowableCol(1, 1);
  }

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Action labels")), 0,
                 wxALIGN_CENTER_VERTICAL);
  if (m_isChance) {
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Probability")), 0,
                   wxALIGN_CENTER_VERTICAL);
  }
  gridSizer->AddSpacer(1);

  const size_t activeCount = m_list.ActiveCount();
  const wxSize buttonSize(FromDIP(c_rowButtonWidth), -1);
  const int buttonGap = FromDIP(c_spaceTight);

  for (size_t i = 0; i < m_list.Size(); i++) {
    const auto &row = m_list.GetRow(i);

    auto *labelCtrl = new LabelTextCtrl(this, wxID_ANY, wxString::FromUTF8(row.label));
    m_labelCtrls.push_back(labelCtrl);
    if (row.isDeleted) {
      wxFont strikeFont = labelCtrl->GetFont();
      strikeFont.SetStrikethrough(true);
      labelCtrl->SetFont(strikeFont);
      labelCtrl->Disable();
    }
    else {
      StyleLabelRow(labelCtrl, row.isNew, row.stableLabel, row.label);
    }
    labelCtrl->Bind(wxEVT_TEXT, [this, i](wxCommandEvent &p_event) {
      if (!m_rebuilding && i < m_list.Size()) {
        m_list.SetLabel(i, m_labelCtrls[i]->GetValue().ToStdString(wxConvUTF8));
        const auto &changedRow = m_list.GetRow(i);
        StyleLabelRow(m_labelCtrls[i], changedRow.isNew, changedRow.stableLabel, changedRow.label);
        NotifyChanged();
      }
      p_event.Skip();
    });
    labelCtrl->Bind(wxEVT_KILL_FOCUS, [this, i](wxFocusEvent &p_event) {
      if (!m_rebuilding && i < m_list.Size()) {
        m_list.SetLabel(i, m_labelCtrls[i]->GetNormalizedValue().ToStdString(wxConvUTF8));
        const auto &changedRow = m_list.GetRow(i);
        StyleLabelRow(m_labelCtrls[i], changedRow.isNew, changedRow.stableLabel, changedRow.label);
        NotifyChanged();
      }
      p_event.Skip();
    });
    gridSizer->Add(labelCtrl, 1, wxEXPAND);

    if (m_isChance) {
      auto *probCtrl =
          new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0,
                         NumberValidator(row.payload.get(), Rational(0), Rational(1)));
      probCtrl->Enable(!row.isDeleted);
      probCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent &p_event) {
        if (!m_rebuilding) {
          NotifyChanged();
        }
        p_event.Skip();
      });
      m_probCtrls.push_back(probCtrl);
      gridSizer->Add(probCtrl, 1, wxEXPAND);
    }

    auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    if (!row.isDeleted) {
      auto *upButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2191), wxDefaultPosition, buttonSize);
      upButton->Enable(i > 0);
      upButton->Bind(wxEVT_BUTTON,
                     [this, i](wxCommandEvent &) { MoveActionUp(static_cast<int>(i)); });
      buttonSizer->Add(upButton, 0, wxLEFT, buttonGap);

      auto *downButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2193), wxDefaultPosition, buttonSize);
      downButton->Enable(i + 1 < m_list.Size());
      downButton->Bind(wxEVT_BUTTON,
                       [this, i](wxCommandEvent &) { MoveActionDown(static_cast<int>(i)); });
      buttonSizer->Add(downButton, 0, wxLEFT, buttonGap);
    }
    else {
      // Keep the delete/restore button aligned under its counterpart in other rows, in
      // place of the Up/Down buttons a deleted row no longer has.
      buttonSizer->AddSpacer(buttonSize.GetWidth() + buttonGap);
      buttonSizer->AddSpacer(buttonSize.GetWidth() + buttonGap);
    }

    auto *deleteButton =
        new wxButton(this, wxID_ANY, row.isDeleted ? wxUniChar(0x21BA) : wxUniChar(0x2715),
                     wxDefaultPosition, buttonSize);
    deleteButton->SetToolTip(row.isDeleted ? _("Restore action") : _("Delete action"));
    deleteButton->Enable(row.isDeleted || activeCount > 1);
    deleteButton->Bind(wxEVT_BUTTON,
                       [this, i](wxCommandEvent &) { ToggleDeleted(static_cast<int>(i)); });
    buttonSizer->Add(deleteButton, 0, wxLEFT, buttonGap);

    gridSizer->Add(buttonSizer, 0, wxALIGN_CENTER_VERTICAL);
  }

  // A trailing row with only its button-column cell populated, so the "+" control lands
  // directly below the last row's Up/Down/Delete buttons rather than as a separate control
  // elsewhere in the dialog.
  gridSizer->AddSpacer(1);
  if (m_isChance) {
    gridSizer->AddSpacer(1);
  }
  auto *addButton = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, buttonSize);
  addButton->SetToolTip(_("Add action"));
  addButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { AddAction(); });
  auto *addSizer = new wxBoxSizer(wxHORIZONTAL);
  addSizer->Add(addButton, 0, wxLEFT, buttonGap);
  gridSizer->Add(addSizer, 0, wxALIGN_CENTER_VERTICAL);

  m_topSizer->Add(gridSizer, 1, wxEXPAND);
  FitInside();
  Layout();
  m_rebuilding = false;
  // No NotifyChanged() call here -- see m_ready's comment. Callers that mutate m_list are
  // responsible for notifying once Rebuild() returns.
}

void ActionPanel::ScheduleRebuild(bool p_scrollToBottom)
{
  m_rebuilding = true;
  CallAfter([this, p_scrollToBottom]() {
    Rebuild();
    if (p_scrollToBottom) {
      ScrollToBottom();
    }
    NotifyChanged();
  });
}

void ActionPanel::AddAction()
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  m_list.AddNew("", m_isChance ? std::make_unique<wxString>("0") : nullptr);
  ScheduleRebuild(true);
}

void ActionPanel::ScrollToBottom()
{
  int xUnit, yUnit;
  GetScrollPixelsPerUnit(&xUnit, &yUnit);
  if (yUnit > 0) {
    Scroll(0, GetVirtualSize().GetHeight() / yUnit);
  }
}

void ActionPanel::ToggleDeleted(int p_index)
{
  if (!m_rebuilding && m_list.ToggleDeleted(p_index)) {
    ScheduleRebuild();
  }
}

void ActionPanel::MoveActionUp(int p_index)
{
  if (!m_rebuilding && m_list.MoveUp(p_index)) {
    ScheduleRebuild();
  }
}

void ActionPanel::MoveActionDown(int p_index)
{
  if (!m_rebuilding && m_list.MoveDown(p_index)) {
    ScheduleRebuild();
  }
}

wxString ActionPanel::CheckLabels()
{
  return ValidateLabels(m_list.Validate(), m_labelCtrls, m_defaultBg,
                        _("Action labels cannot be empty."),
                        m_isChance ? _("Action labels must be unique within the event.")
                                   : _("Action labels must be unique within the infoset."));
}

wxString ActionPanel::CheckProbabilities()
{
  if (!m_isChance) {
    return {};
  }

  std::vector<Number> probs;
  wxString message;
  for (size_t i = 0; i < m_probCtrls.size(); i++) {
    bool invalid = false;
    if (!m_list.GetRow(i).isDeleted) {
      try {
        probs.emplace_back(m_probCtrls[i]->GetValue().ToStdString());
      }
      catch (const std::exception &) {
        invalid = true;
      }
    }
    m_probCtrls[i]->SetBackgroundColour(invalid ? InvalidFieldBackground() : m_defaultBg);
    m_probCtrls[i]->Refresh();
    if (invalid && message.empty()) {
      message = _("Probabilities must be valid numbers.");
    }
  }
  if (!message.empty()) {
    return message;
  }

  try {
    ValidateDistribution(probs);
  }
  catch (const std::exception &) {
    for (size_t i = 0; i < m_probCtrls.size(); i++) {
      if (!m_list.GetRow(i).isDeleted) {
        m_probCtrls[i]->SetBackgroundColour(InvalidFieldBackground());
        m_probCtrls[i]->Refresh();
      }
    }
    return _("Probabilities must be nonnegative numbers summing to one.");
  }
  return {};
}

//======================================================================
//                      class EditMoveDialog
//======================================================================

EditMoveDialog::EditMoveDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                               const GameInfoset &p_infoset)
  : wxDialog(p_parent, wxID_ANY, _("Move properties"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc), m_infoset(p_infoset)
{
  // The dialog keeps the platform's normal dialog-chrome background; only the header panel
  // (explicitly BTNFACE, just below) and the action list itself (explicitly WINDOW, in
  // ActionPanel's own constructor) are given a different one, so the action list reads as the
  // one light "surface" rather than the whole dialog being white.
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  // Header: a shaded strip, divided from the action list below -- the same treatment
  // AppendMovePopup uses for its "Append/Insert move for <player>" sentence, applied here to
  // the analogous "which player, which information set" fields.
  m_headerPanel = new wxPanel(this);
  m_headerPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

  auto *headerSizer = new wxBoxSizer(wxVERTICAL);

  auto *sentenceSizer = new wxBoxSizer(wxHORIZONTAL);
  sentenceSizer->Add(new wxStaticText(m_headerPanel, wxID_ANY, _("Move for")), 0,
                     wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(c_spaceRelated));

  // wxBitmapComboBox carries each player's colour swatch as part of the dropdown itself
  // (cross-platform -- Windows/GTK/macOS all implement it), rather than needing a separate
  // wxStaticBitmap kept in sync alongside a plain wxChoice.
  m_player = new wxBitmapComboBox(m_headerPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                  wxDefaultSize, 0, nullptr, wxCB_READONLY);
  if (p_infoset->IsChanceInfoset()) {
    m_player->Append(_("Chance"),
                     MakeColorSwatch(p_doc->GetPlayerColor(p_infoset->GetGame()->GetChance())));
    m_player->SetSelection(0);
    m_player->Disable();
  }
  else {
    for (const auto &player : p_infoset->GetGame()->GetPlayers()) {
      wxString label;
      label << player->GetNumber() << ": " << player->GetLabel();
      m_player->Append(label, MakeColorSwatch(p_doc->GetPlayerColor(player)));
    }
    m_player->SetSelection(p_infoset->GetPlayer()->GetNumber() - 1);
  }
  sentenceSizer->Add(m_player, 1, wxALIGN_CENTER_VERTICAL | wxEXPAND);

  headerSizer->Add(sentenceSizer, 0, wxEXPAND | wxALL, FromDIP(c_spaceMargin));

  {
    wxString label;
    label << _("Shared by ") << p_infoset->GetMembers().size() << _(" node(s)");
    auto *memberText = new wxStaticText(m_headerPanel, wxID_ANY, label);
    memberText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    wxFont memberFont = memberText->GetFont();
    memberFont.SetPointSize(memberFont.GetPointSize() - 1);
    memberText->SetFont(memberFont);
    headerSizer->Add(memberText, 0, wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(c_spaceMargin));
  }

  auto *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(m_headerPanel, wxID_STATIC,
                                   p_infoset->IsChanceInfoset() ? _("Event label")
                                                                : _("Information set label")),
                  0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(c_spaceRelated));
  m_infosetLabel =
      new LabelTextCtrl(m_headerPanel, wxID_ANY, wxString::FromUTF8(p_infoset->GetLabel()));
  m_infosetLabelDefaultBg = m_infosetLabel->GetBackgroundColour();
  m_infosetLabel->Bind(wxEVT_TEXT, [this](wxCommandEvent &p_event) {
    UpdateValidation();
    p_event.Skip();
  });
  m_infosetLabel->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent &p_event) {
    UpdateValidation();
    p_event.Skip();
  });
  labelSizer->Add(m_infosetLabel, 1, wxEXPAND);
  headerSizer->Add(labelSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(c_spaceMargin));

  m_headerPanel->SetSizer(headerSizer);
  topSizer->Add(m_headerPanel, 0, wxEXPAND);

  topSizer->Add(new wxStaticLine(this), 0, wxEXPAND);

  // A white surface, like the header panel is a shaded one, so the margin around the action
  // list reads as part of that white surface rather than as a strip of the dialog's own grey
  // showing through: putting the padding directly on topSizer (the dialog itself) would have
  // left the margin grey while only the list inside it was white.
  auto *actionAreaPanel = new wxPanel(this);
  actionAreaPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
  auto *actionAreaSizer = new wxBoxSizer(wxVERTICAL);
  // ActionPanel sizes itself to fit its own content (see its constructor); don't override
  // that with a fixed estimate here, which would go stale as soon as the row count changes.
  m_actionPanel = new ActionPanel(actionAreaPanel, p_infoset, [this]() { UpdateValidation(); });
  actionAreaSizer->Add(m_actionPanel, 1, wxEXPAND | wxALL, FromDIP(c_spaceMargin));
  actionAreaPanel->SetSizer(actionAreaSizer);
  topSizer->Add(actionAreaPanel, 1, wxEXPAND);

  topSizer->Add(new wxStaticLine(this), 0, wxEXPAND);

  // The error text and buttons sit beneath the full-width panels above, so this dialog lays
  // out its own margins rather than using SetDialogLayout().
  m_errorText = NewErrorText(this);
  topSizer->Add(m_errorText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(c_spaceMargin));
  topSizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL,
                FromDIP(c_spaceMargin));

  SetSizer(topSizer);
  Bind(wxEVT_BUTTON, &EditMoveDialog::OnOK, this, wxID_OK);

  // Hides m_errorText when there's nothing to report (there never is yet, at this point) --
  // done before the size calculations below so the empty state doesn't reserve dead space
  // for a message that isn't there, the extra gap above OK/Cancel that this was chasing.
  UpdateValidation();

  topSizer->SetSizeHints(this);
  SetSize(GetBestSize());
  SetMinSize(GetSize());
  CenterOnParent();
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
  m_infosetLabel->SetBackgroundColour(infosetValid ? m_infosetLabelDefaultBg
                                                   : InvalidFieldBackground());
  m_infosetLabel->Refresh();
  if (!infosetValid) {
    message = m_infoset->IsChanceInfoset()
                  ? _("Event label must be unique.")
                  : _("Information set label must be unique for the player.");
  }

  const wxString actionMessage = m_actionPanel->CheckLabels();
  if (message.empty()) {
    message = actionMessage;
  }
  const wxString probabilityMessage = m_actionPanel->CheckProbabilities();
  if (message.empty()) {
    message = probabilityMessage;
  }

  m_errorText->SetLabel(message);
  m_errorText->Show(!message.empty());
  RefreshDialogMinSize(this);
  if (auto *ok = FindWindow(wxID_OK)) {
    ok->Enable(message.empty());
  }
}

void EditMoveDialog::OnOK(wxCommandEvent &p_event)
{
  UpdateValidation();
  if (!m_errorText->GetLabel().empty()) {
    wxBell();
    return;
  }
  p_event.Skip();
}

EditMove EditMoveDialog::GetEdit() const
{
  const auto &actions = m_actionPanel->GetEdit();
  return {.infoset = m_infoset,
          .infosetLabel = m_infosetLabel->GetNormalizedValue().ToStdString(wxConvUTF8),
          .player = m_infoset->IsChanceInfoset()
                        ? GamePlayer(nullptr)
                        : m_infoset->GetGame()->GetPlayer(m_player->GetSelection() + 1),
          .stableLabels = actions.GetStableLabels(),
          .labels = actions.GetLabels(),
          .probs = m_actionPanel->GetActiveProbs()};
}

} // namespace Gambit::GUI
