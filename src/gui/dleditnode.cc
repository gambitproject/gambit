//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#include "gambit.h"
#include "dleditnode.h"

//======================================================================
//                      class dialogEditNode
//======================================================================

dialogEditNode::dialogEditNode(wxWindow *p_parent, const Gambit::GameNode &p_node)
  : wxDialog(p_parent, wxID_ANY, _("Node properties"), wxDefaultPosition), m_node(p_node)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Node label")), 0, wxALL | wxCENTER, 5);
  m_nodeName =
      new wxTextCtrl(this, wxID_ANY, wxString(m_node->GetLabel().c_str(), *wxConvCurrent));
  labelSizer->Add(m_nodeName, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 5);

  auto *infosetSizer = new wxBoxSizer(wxHORIZONTAL);
  infosetSizer->Add(new wxStaticText(this, wxID_STATIC, _("Information set")), 0, wxALL | wxCENTER,
                    5);
  m_infoset = new wxChoice(this, wxID_ANY);
  if (p_node->NumChildren() > 0) {
    m_infoset->Append(_("New information set"));
    if (p_node->GetInfoset()->IsChanceInfoset()) {
      int selection = 0;
      for (const auto &infoset : p_node->GetGame()->GetChance()->GetInfosets()) {
        if (infoset->NumActions() == p_node->NumChildren()) {
          m_infosetList.push_back(infoset);
          m_infoset->Append(wxString::Format(_("Chance infoset %d"), infoset->GetNumber()));
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
          if (infoset->NumActions() == p_node->NumChildren()) {
            m_infosetList.push_back(infoset);
            m_infoset->Append(wxString::Format(_("Player %d, Infoset %d"), player->GetNumber(),
                                               infoset->GetNumber()));
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
  infosetSizer->Add(m_infoset, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(infosetSizer, 0, wxALL | wxEXPAND, 5);

  auto *subgameSizer = new wxBoxSizer(wxVERTICAL);
  if (!p_node->GetParent()) {
    subgameSizer->Add(new wxStaticText(this, wxID_STATIC, _("This is the root node of the tree")),
                      0, wxALL | wxCENTER, 5);
  }
  else if (p_node->IsSubgameRoot()) {
    subgameSizer->Add(
        new wxStaticText(this, wxID_STATIC, _("This is the root of a proper subgame")), 0,
        wxALL | wxCENTER, 5);
  }
  topSizer->Add(subgameSizer, 0, wxALL | wxCENTER, 5);

  auto *outcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  outcomeSizer->Add(new wxStaticText(this, wxID_STATIC, _("Outcome")), 0, wxALL | wxCENTER, 5);
  m_outcome = new wxChoice(this, wxID_ANY);
  m_outcome->Append(_("(null)"));
  m_outcome->SetSelection(0);
  const Gambit::Game efg = p_node->GetGame();
  for (size_t outc = 1; outc <= efg->NumOutcomes(); outc++) {
    const Gambit::GameOutcome outcome = efg->GetOutcome(outc);
    std::string item = Gambit::lexical_cast<std::string>(outc) + ": " + outcome->GetLabel();
    if (item.empty()) {
      item = "Outcome" + Gambit::lexical_cast<std::string>(outc);
    }

    item += (" (" + outcome->GetPayoff<std::string>(efg->GetPlayer(1)) + ", " +
             outcome->GetPayoff<std::string>(efg->GetPlayer(2)));
    if (efg->NumPlayers() > 2) {
      item += ", " + outcome->GetPayoff<std::string>(efg->GetPlayer(3));
      if (efg->NumPlayers() > 3) {
        item += ",...)";
      }
      else {
        item += ")";
      }
    }
    else {
      item += ")";
    }

    m_outcome->Append(wxString(item.c_str(), *wxConvCurrent));
    if (m_node->GetOutcome() == outcome) {
      m_outcome->SetSelection(outc);
    }
  }
  outcomeSizer->Add(m_outcome, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(outcomeSizer, 0, wxALL | wxEXPAND, 5);

  auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  auto *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

Gambit::GameInfoset dialogEditNode::GetInfoset() const
{
  if (m_infoset->GetSelection() == 0) {
    return nullptr;
  }
  else {
    return m_infosetList[m_infoset->GetSelection()];
  }
}
