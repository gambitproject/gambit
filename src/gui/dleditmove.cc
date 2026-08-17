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

#include <functional>

#include "gambit.h"
#include "dleditmove.h"
#include "valnumber.h"
#include "editlabel.h"

namespace Gambit::GUI {

namespace {
const wxColour kInvalidLabelBg(255, 220, 220);
} // namespace

class ActionPanel final : public wxScrolledWindow {
  std::vector<wxString> m_actionProbValues;
  std::vector<LabelTextCtrl *> m_actionLabels;
  std::vector<wxTextCtrl *> m_actionProbs;
  wxColour m_defaultBg;

public:
  ActionPanel(wxWindow *p_parent, const GameInfoset &p_infoset,
              const std::function<void()> &p_onChanged);

  int NumActions() const { return static_cast<int>(m_actionLabels.size()); }

  wxString GetActionLabel(int p_act) const;
  Array<Number> GetActionProbs() const;

  // Highlights any empty or duplicate action labels, and returns a description of the
  // first problem found, or an empty string if all labels are valid.
  wxString ValidateLabels();
};

ActionPanel::ActionPanel(wxWindow *p_parent, const GameInfoset &p_infoset,
                         const std::function<void()> &p_onChanged)
  : wxScrolledWindow(p_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxVSCROLL | wxTAB_TRAVERSAL)
{
  const bool isChance = p_infoset->IsChanceInfoset();

  if (isChance) {
    m_actionProbValues.reserve(p_infoset->GetActions().size());
    m_actionProbs.reserve(p_infoset->GetActions().size());
  }
  m_actionLabels.reserve(p_infoset->GetActions().size());

  const int numColumns = isChance ? 3 : 2;

  auto *gridSizer = new wxFlexGridSizer(numColumns, 5, 10);
  gridSizer->AddGrowableCol(1, 1);
  if (isChance) {
    gridSizer->AddGrowableCol(2, 1);
  }

  gridSizer->AddSpacer(1);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Label")), 0, wxALIGN_CENTER_VERTICAL);
  if (isChance) {
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Probability")), 0,
                   wxALIGN_CENTER_VERTICAL);
  }

  for (const auto &action : p_infoset->GetActions()) {
    wxString number;
    number << action->GetNumber();

    gridSizer->Add(new wxStaticText(this, wxID_STATIC, number), 0,
                   wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

    auto *name = new LabelTextCtrl(this, wxID_ANY, wxString::FromUTF8(action->GetLabel()));
    m_actionLabels.push_back(name);
    name->Bind(wxEVT_TEXT, [p_onChanged](wxCommandEvent &p_event) {
      p_onChanged();
      p_event.Skip();
    });
    name->Bind(wxEVT_KILL_FOCUS, [p_onChanged](wxFocusEvent &p_event) {
      p_onChanged();
      p_event.Skip();
    });
    gridSizer->Add(name, 1, wxEXPAND);

    if (isChance) {
      m_actionProbValues.emplace_back(
          static_cast<std::string>(p_infoset->GetActionProb(action)).c_str(), *wxConvCurrent);

      auto *probability =
          new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0,
                         NumberValidator(&m_actionProbValues.back(), Rational(0), Rational(1)));

      m_actionProbs.push_back(probability);
      gridSizer->Add(probability, 1, wxEXPAND);
    }
  }

  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);
  SetSizer(topSizer);

  SetScrollRate(0, FromDIP(10));
  FitInside();

  const wxSize bestSize = topSizer->CalcMin();
  SetMinSize(wxSize(FromDIP(isChance ? 400 : 300), std::min(bestSize.GetHeight(), FromDIP(250))));

  if (!m_actionLabels.empty()) {
    m_defaultBg = m_actionLabels.front()->GetBackgroundColour();
  }
}

wxString ActionPanel::GetActionLabel(int p_act) const
{
  return m_actionLabels.at(p_act - 1)->GetNormalizedValue();
}

Array<Number> ActionPanel::GetActionProbs() const
{
  Array<Number> probs(NumActions());
  for (int act = 1; act <= NumActions(); act++) {
    probs[act] = Number(m_actionProbs.at(act - 1)->GetValue().ToStdString());
  }
  return probs;
}

wxString ActionPanel::ValidateLabels()
{
  const int numActions = NumActions();
  wxString message;

  for (int act = 1; act <= numActions; act++) {
    const wxString value = m_actionLabels.at(act - 1)->GetValue();
    bool invalid = value.empty();
    for (int other = 1; !invalid && other <= numActions; other++) {
      if (other != act && m_actionLabels.at(other - 1)->GetValue() == value) {
        invalid = true;
      }
    }

    m_actionLabels.at(act - 1)->SetBackgroundColour(invalid ? kInvalidLabelBg : m_defaultBg);
    m_actionLabels.at(act - 1)->Refresh();

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

EditMoveDialog::EditMoveDialog(wxWindow *p_parent, const GameNode &p_node)
  : wxDialog(p_parent, wxID_ANY, _("Move properties"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_node(p_node)
{
  const GameInfoset infoset = p_node->GetInfoset();

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Information set label")), 0,
                  wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_infosetLabel =
      new LabelTextCtrl(this, wxID_ANY, wxString::FromUTF8(p_node->GetInfosetLabel()));
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
    label << _("Number of members: ") << infoset->GetMembers().size();
    topSizer->Add(new wxStaticText(this, wxID_STATIC, label), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
  }

  auto *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, _("Belongs to player")), 0,
                   wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_player = new wxChoice(this, wxID_ANY);
  if (p_node->IsChanceInfoset()) {
    m_player->Append(_("Chance"));
    m_player->SetSelection(0);
    m_player->Disable();
  }
  else {
    for (const auto &player : p_node->GetGame()->GetPlayers()) {
      wxString label;
      label << player->GetNumber() << ": " << player->GetLabel();
      m_player->Append(label);
    }
    m_player->SetSelection(p_node->GetPlayer()->GetNumber() - 1);
  }
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxEXPAND);

  auto *actionBoxSizer =
      new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, _("Actions")), wxVERTICAL);
  m_actionPanel = new ActionPanel(this, infoset, [this]() { UpdateValidation(); });

  const int visibleRows = std::min(static_cast<int>(infoset->GetActions().size()), 10);
  const int rowHeight = FromDIP(32);
  const int headerHeight = FromDIP(28);
  m_actionPanel->SetMinSize(wxSize(FromDIP(p_node->IsChanceInfoset() ? 400 : 300),
                                   headerHeight + visibleRows * rowHeight));

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
    for (const auto &infoset : m_node->GetPlayer()->GetInfosets()) {
      if (!m_node->SameInfoset(infoset) && infoset->GetLabel() == infosetLabel) {
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

  if (!m_node->IsChanceInfoset()) {
    p_event.Skip();
    return;
  }
  try {
    ValidateDistribution(m_actionPanel->GetActionProbs());
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

wxString EditMoveDialog::GetActionLabel(int p_act) const
{
  return m_actionPanel->GetActionLabel(p_act);
}

Array<Number> EditMoveDialog::GetActionProbs() const { return m_actionPanel->GetActionProbs(); }

} // namespace Gambit::GUI
