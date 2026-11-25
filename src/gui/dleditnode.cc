//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dleditnode.cc
// Dialog for viewing and editing properties of a node
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
#include "games.h"
#include "games/gametree.h"
#include "dleditnode.h"
#include "dialogstyle.h"

namespace Gambit::GUI {
//======================================================================
//                      class EditNodeDialog
//======================================================================

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
EditNodeDialog::EditNodeDialog(wxWindow *p_parent, const GameNode &p_node)
  : wxDialog(p_parent, wxID_ANY, _("Node properties"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_node(p_node)
{
  auto *formSizer = NewFormSizer(this);

  formSizer->Add(new wxStaticText(this, wxID_STATIC, _("Node label")), 0, wxALIGN_CENTER_VERTICAL);
  m_nodeLabel = new LabelTextCtrl(this, wxID_ANY, wxString::FromUTF8(m_node->GetLabel()));
  formSizer->Add(m_nodeLabel, 1, wxEXPAND);

  const bool isChance = !p_node->IsTerminal() && p_node->GetInfoset()->IsChanceInfoset();
  formSizer->Add(new wxStaticText(this, wxID_STATIC, isChance ? _("Event") : _("Information set")),
                 0, wxALIGN_CENTER_VERTICAL);
  m_infoset = new wxChoice(this, wxID_ANY);
  if (!p_node->IsTerminal()) {
    m_infoset->Append(isChance ? _("New event") : _("New information set"));
    if (isChance) {
      int selection = 0;
      for (const auto &infoset : p_node->GetGame()->GetChance()->GetInfosets()) {
        if (infoset->GetActions().size() == p_node->GetChildren().size()) {
          m_infosetList.push_back(infoset);
          wxString label;
          label << _("Event ") << infoset->GetNumber();
          m_infoset->Append(label);
          if (infoset == p_node->GetInfoset()) {
            selection = m_infosetList.size();
          }
        }
      }
      m_infoset->SetSelection(selection);
    }
    else {
      int selection = 0;
      for (const auto &player : p_node->GetGame()->GetPlayers()) {
        for (const auto &infoset : player->GetInfosets()) {
          if (infoset->GetActions().size() == p_node->GetChildren().size()) {
            m_infosetList.push_back(infoset);
            wxString label;
            label << _("Player ") << player->GetNumber() << _(", information set ")
                  << infoset->GetNumber();
            m_infoset->Append(label);
            if (infoset == p_node->GetInfoset()) {
              selection = m_infosetList.size();
            }
          }
        }
      }
      m_infoset->SetSelection(selection);
    }
  }
  else {
    m_infoset->Append(_("(none)"));
    m_infoset->SetSelection(0);
    m_infoset->Enable(false);
  }
  formSizer->Add(m_infoset, 1, wxEXPAND);

  formSizer->Add(new wxStaticText(this, wxID_STATIC, _("Outcome")), 0, wxALIGN_CENTER_VERTICAL);
  m_outcome = new wxChoice(this, wxID_ANY);
  m_outcome->Append(_("(null)"));
  m_outcome->SetSelection(0);
  const Game efg = p_node->GetGame();
  for (const auto &outcome : efg->GetOutcomes()) {
    std::string item =
        lexical_cast<std::string>(outcome->GetNumber()) + ": " + outcome->GetLabel();
    if (item.empty()) {
      item = "Outcome" + lexical_cast<std::string>(outcome->GetNumber());
    }

    item += (" (" + outcome->GetPayoff<std::string>(efg->GetPlayer(1)) + ", " +
             outcome->GetPayoff<std::string>(efg->GetPlayer(2)));
    if (efg->NumPlayers() > 2) {
      item += ", " + outcome->GetPayoff<std::string>(efg->GetPlayer(3));
      if (efg->NumPlayers() > 3) {
        item += ",...)";
      }
      else {
        item += ')';
      }
    }
    else {
      item += ')';
    }

    m_outcome->Append(wxString::FromUTF8(item));
    if (m_node->GetOutcome() == outcome) {
      m_outcome->SetSelection(outcome->GetNumber());
    }
  }
  formSizer->Add(m_outcome, 1, wxEXPAND);

  auto *contentSizer = new wxBoxSizer(wxVERTICAL);
  contentSizer->Add(formSizer, 0, wxEXPAND);
  if (!p_node->GetParent()) {
    contentSizer->Add(new wxStaticText(this, wxID_STATIC, _("This is the root node of the tree")),
                      0, wxTOP, FromDIP(c_spaceMargin));
  }
  else if (p_node->IsSubgameRoot()) {
    contentSizer->Add(
        new wxStaticText(this, wxID_STATIC, _("This is the root of a proper subgame")), 0, wxTOP,
        FromDIP(c_spaceMargin));
  }

  m_errorText = NewErrorText(this);
  contentSizer->Add(m_errorText, 0, wxEXPAND | wxTOP, FromDIP(c_spaceRelated));

  SetDialogLayout(this, contentSizer, wxOK | wxCANCEL);

  m_defaultLabelBg = m_nodeLabel->GetBackgroundColour();
  m_nodeLabel->Bind(wxEVT_TEXT, [this](wxCommandEvent &p_event) {
    UpdateValidation();
    p_event.Skip();
  });
  UpdateValidation();

  Bind(wxEVT_BUTTON, &EditNodeDialog::OnOK, this, wxID_OK);
}

void EditNodeDialog::UpdateValidation()
{
  const wxString nodeLabel = m_nodeLabel->GetValue().Strip(wxString::trailing);
  bool duplicate = false;
  if (!nodeLabel.empty()) {
    for (const auto &node : m_node->GetGame()->GetNodes()) {
      if (node != m_node && wxString::FromUTF8(node->GetLabel()) == nodeLabel) {
        duplicate = true;
        break;
      }
    }
  }
  m_nodeLabel->SetBackgroundColour(duplicate ? InvalidFieldBackground() : m_defaultLabelBg);
  m_nodeLabel->Refresh();
  m_errorText->SetLabel(duplicate ? _("Node label must be unique in the game.") : wxString());
  m_errorText->Show(duplicate);
  if (auto *ok = FindWindow(wxID_OK)) {
    ok->Enable(!duplicate);
  }
  RefreshDialogMinSize(this);
}

void EditNodeDialog::OnOK(wxCommandEvent &p_event)
{
  m_nodeLabel->GetNormalizedValue();
  UpdateValidation();
  if (m_errorText->IsShown()) {
    wxBell();
    return;
  }
  p_event.Skip();
}

GameInfoset EditNodeDialog::GetInfoset() const
{
  if (m_infoset->GetSelection() == 0) {
    return nullptr;
  }
  return m_infosetList[m_infoset->GetSelection()];
}
} // namespace Gambit::GUI
