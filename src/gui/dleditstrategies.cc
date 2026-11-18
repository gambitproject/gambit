//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dleditstrategies.cc
// Dialog for viewing and editing the strategies of a player
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

#include <algorithm>
#include <functional>

#include "gambit.h"
#include "dleditstrategies.h"
#include "editlabel.h"

namespace Gambit::GUI {

// An editable, variable-length list of a player's strategies, colour-coded like a
// source-control diff (added/renamed/deleted). The rows are held in a `LabelListEdit`; this
// panel draws them and forwards the user's actions to it.
class StrategyPanel final : public wxScrolledWindow {
  LabelListEdit<> m_list;
  // One per row of m_list, recreated by each Rebuild()
  std::vector<LabelTextCtrl *> m_labelCtrls;
  wxBoxSizer *m_topSizer;
  wxColour m_defaultBg;
  std::function<void()> m_onChanged;
  // False until the constructor completes, so Rebuild()'s initial call can't notify the
  // owning dialog through its m_strategyPanel pointer before that's been assigned.
  bool m_ready = false;
  // True from the moment a row mutation starts until its (deferred) Rebuild() finishes.
  // Destroying/rebuilding row controls can fire a focus-kill event on whatever control had
  // focus, reentrantly, while m_list or the controls are mid-mutation; handlers that touch
  // either must no-op while this is true rather than act on stale state.
  bool m_rebuilding = false;

  void Rebuild();
  // Rebuild() would destroy the very button whose click is invoking a row mutation, while its
  // handler is still on the call stack -- unsafe in wx. CallAfter() defers it to once this
  // event finishes dispatching; raising m_rebuilding now (not just once Rebuild() starts)
  // covers the gap where m_list is already mutated but the stale old controls are still alive.
  void ScheduleRebuild();
  void NotifyChanged() const
  {
    if (m_ready) {
      m_onChanged();
    }
  }

public:
  StrategyPanel(wxWindow *p_parent, const GamePlayer &p_player,
                const std::function<void()> &p_onChanged);

  // The rows as edited, with each label as normalized in its control
  const LabelListEdit<> &GetEdit();

  void AddStrategy();
  // Marks an existing row deleted (or restores an already-deleted one); removes a row added
  // in this dialog outright. A no-op if this would leave no active (non-deleted) rows.
  void ToggleDeleted(int p_index);
  void MoveStrategyUp(int p_index);
  void MoveStrategyDown(int p_index);

  // Highlights any empty or duplicate strategy labels among active (non-deleted) rows, and
  // returns a description of the first problem found, or an empty string if all are valid.
  wxString CheckLabels();
};

StrategyPanel::StrategyPanel(wxWindow *p_parent, const GamePlayer &p_player,
                             const std::function<void()> &p_onChanged)
  : wxScrolledWindow(p_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxVSCROLL | wxTAB_TRAVERSAL),
    m_topSizer(new wxBoxSizer(wxVERTICAL)), m_onChanged(p_onChanged)
{
  for (const auto &strategy : p_player->GetStrategies()) {
    m_list.AddExisting(strategy->GetLabel());
  }

  SetSizer(m_topSizer);
  SetScrollRate(0, FromDIP(10));

  Rebuild();

  const wxSize bestSize = m_topSizer->CalcMin();
  SetMinSize(wxSize(FromDIP(360), std::min(bestSize.GetHeight(), FromDIP(250))));

  if (!m_labelCtrls.empty()) {
    m_defaultBg = m_labelCtrls.front()->GetBackgroundColour();
  }

  m_ready = true;
}

const LabelListEdit<> &StrategyPanel::GetEdit()
{
  for (size_t i = 0; i < m_labelCtrls.size(); i++) {
    m_list.SetLabel(i, m_labelCtrls[i]->GetNormalizedValue().ToStdString(wxConvUTF8));
  }
  return m_list;
}

void StrategyPanel::Rebuild()
{
  m_rebuilding = true;
  m_topSizer->Clear(true); // destroys the previous row controls; the rows themselves are in
                           // m_list, which they don't own
  m_labelCtrls.clear();

  auto *gridSizer = new wxFlexGridSizer(3, 5, 10);
  gridSizer->AddGrowableCol(1, 1);

  gridSizer->AddSpacer(1);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Label")), 0, wxALIGN_CENTER_VERTICAL);
  gridSizer->AddSpacer(1);

  const size_t activeCount = m_list.ActiveCount();
  const wxSize buttonSize(FromDIP(28), -1);

  for (size_t i = 0; i < m_list.Size(); i++) {
    const auto &row = m_list.GetRow(i);

    wxString number;
    number << (i + 1);
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, number), 0,
                   wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

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

    auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    if (!row.isDeleted) {
      auto *upButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2191), wxDefaultPosition, buttonSize);
      upButton->Enable(i > 0);
      upButton->Bind(wxEVT_BUTTON,
                     [this, i](wxCommandEvent &) { MoveStrategyUp(static_cast<int>(i)); });
      buttonSizer->Add(upButton, 0, wxLEFT, 2);

      auto *downButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2193), wxDefaultPosition, buttonSize);
      downButton->Enable(i + 1 < m_list.Size());
      downButton->Bind(wxEVT_BUTTON,
                       [this, i](wxCommandEvent &) { MoveStrategyDown(static_cast<int>(i)); });
      buttonSizer->Add(downButton, 0, wxLEFT, 2);
    }
    else {
      // Keep the delete/restore button aligned under its counterpart in other rows, in
      // place of the Up/Down buttons a deleted row no longer has.
      buttonSizer->AddSpacer(buttonSize.GetWidth() + 2);
      buttonSizer->AddSpacer(buttonSize.GetWidth() + 2);
    }

    auto *deleteButton =
        new wxButton(this, wxID_ANY, row.isDeleted ? wxUniChar(0x21BA) : wxUniChar(0x2715),
                     wxDefaultPosition, buttonSize);
    deleteButton->SetToolTip(row.isDeleted ? _("Restore strategy") : _("Delete strategy"));
    deleteButton->Enable(row.isDeleted || activeCount > 1);
    deleteButton->Bind(wxEVT_BUTTON,
                       [this, i](wxCommandEvent &) { ToggleDeleted(static_cast<int>(i)); });
    buttonSizer->Add(deleteButton, 0, wxLEFT, 2);

    gridSizer->Add(buttonSizer, 0, wxALIGN_CENTER_VERTICAL);
  }

  // A trailing row with only its button-column cell populated, so the "+" control lands
  // directly below the last row's Up/Down/Delete buttons rather than as a separate control
  // elsewhere in the dialog.
  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  auto *addButton = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, buttonSize);
  addButton->SetToolTip(_("Add strategy"));
  addButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { AddStrategy(); });
  auto *addSizer = new wxBoxSizer(wxHORIZONTAL);
  addSizer->Add(addButton, 0, wxLEFT, 2);
  gridSizer->Add(addSizer, 0, wxALIGN_CENTER_VERTICAL);

  m_topSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);
  FitInside();
  Layout();
  m_rebuilding = false;
  // No NotifyChanged() call here -- see m_ready's comment. Callers that mutate m_list are
  // responsible for notifying once Rebuild() returns.
}

void StrategyPanel::ScheduleRebuild()
{
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void StrategyPanel::AddStrategy()
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  m_list.AddNew("");
  ScheduleRebuild();
}

void StrategyPanel::ToggleDeleted(int p_index)
{
  if (!m_rebuilding && m_list.ToggleDeleted(p_index)) {
    ScheduleRebuild();
  }
}

void StrategyPanel::MoveStrategyUp(int p_index)
{
  if (!m_rebuilding && m_list.MoveUp(p_index)) {
    ScheduleRebuild();
  }
}

void StrategyPanel::MoveStrategyDown(int p_index)
{
  if (!m_rebuilding && m_list.MoveDown(p_index)) {
    ScheduleRebuild();
  }
}

wxString StrategyPanel::CheckLabels()
{
  return ValidateLabels(m_list.Validate(), m_labelCtrls, m_defaultBg,
                        _("Strategy labels cannot be empty."),
                        _("Strategy labels must be unique for the player."));
}

//======================================================================
//                    class EditStrategiesDialog
//======================================================================

EditStrategiesDialog::EditStrategiesDialog(wxWindow *p_parent, const GamePlayer &p_player)
  : wxDialog(
        p_parent, wxID_ANY,
        wxString::Format(_("Edit strategies for %s"), wxString::FromUTF8(p_player->GetLabel())),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_player(p_player)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *strategyBoxSizer =
      new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, _("Strategies")), wxVERTICAL);
  // StrategyPanel sizes itself to fit its own content (see its constructor); don't override
  // that with a fixed estimate here, which would go stale as soon as the row count changes.
  m_strategyPanel = new StrategyPanel(this, p_player, [this]() { UpdateValidation(); });

  strategyBoxSizer->Add(m_strategyPanel, 1, wxALL | wxEXPAND, 5);

  topSizer->Add(strategyBoxSizer, 1, wxALL | wxEXPAND, 5);

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

  UpdateValidation();
}

void EditStrategiesDialog::UpdateValidation()
{
  const wxString message = m_strategyPanel->CheckLabels();
  m_errorText->SetLabel(message);
  if (auto *ok = FindWindow(wxID_OK)) {
    ok->Enable(message.empty());
  }
}

const LabelListEdit<> &EditStrategiesDialog::GetStrategies() const
{
  return m_strategyPanel->GetEdit();
}

} // namespace Gambit::GUI
