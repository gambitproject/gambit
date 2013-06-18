//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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
#endif  // WX_PRECOMP
#include "libgambit/libgambit.h"
#include "dleditnode.h"

//======================================================================
//                      class dialogEditNode
//======================================================================

dialogEditNode::dialogEditNode(wxWindow *p_parent, Gambit::GameNode p_node)
  : wxDialog(p_parent, -1, _("Node properties"), wxDefaultPosition), 
    m_node(p_node)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Node label")),
		  0, wxALL | wxCENTER, 5);
  m_nodeName = new wxTextCtrl(this, -1, 
			      wxString(m_node->GetLabel().c_str(), *wxConvCurrent));
  labelSizer->Add(m_nodeName, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *infosetSizer = new wxBoxSizer(wxHORIZONTAL);
  infosetSizer->Add(new wxStaticText(this, wxID_STATIC, _("Information set")),
		    0, wxALL | wxCENTER, 5);
  m_infoset = new wxChoice(this, -1);
  if (p_node->NumChildren() > 0) {
    m_infoset->Append(_("New information set"));
    if (p_node->GetInfoset()->IsChanceInfoset()) {
      int selection = 0;
      for (int iset = 1; iset <= p_node->GetGame()->GetChance()->NumInfosets();
	   iset++) {
	Gambit::GameInfoset infoset = p_node->GetGame()->GetChance()->GetInfoset(iset);
	if (infoset->NumActions() == p_node->NumChildren()) {
	  m_infosetList.Append(infoset);
	  m_infoset->Append(wxString::Format(_("Chance infoset %d"),
					     infoset->GetNumber()));
	  if (infoset == p_node->GetInfoset()) {
	    selection = m_infosetList.Length();
	  }
	}
      }
      m_infoset->SetSelection(selection);
    }
    else {
      int selection = 0;
      for (int pl = 1; pl <= p_node->GetGame()->NumPlayers(); pl++) {
	Gambit::GamePlayer player = p_node->GetGame()->GetPlayer(pl);
	for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	  Gambit::GameInfoset infoset = player->GetInfoset(iset);
	  if (infoset->NumActions() == p_node->NumChildren()) {
	    m_infosetList.Append(infoset);
	    m_infoset->Append(wxString::Format(_("Player %d, Infoset %d"),
					       pl, iset));
	    if (infoset == p_node->GetInfoset()) {
	      selection = m_infosetList.Length();
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

  wxBoxSizer *subgameSizer = new wxBoxSizer(wxVERTICAL);
  if (!p_node->GetParent()) {
    subgameSizer->Add(new wxStaticText(this, wxID_STATIC,
				       _("This is the root node of the tree")),
		      0, wxALL | wxCENTER, 5);
  }
  else if (p_node->IsSubgameRoot()) {
    subgameSizer->Add(new wxStaticText(this, wxID_STATIC,
				       _("This is the root of a proper subgame")),
		      0, wxALL | wxCENTER, 5);
  }
  topSizer->Add(subgameSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *outcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  outcomeSizer->Add(new wxStaticText(this, wxID_STATIC, _("Outcome")),
		    0, wxALL | wxCENTER, 5);
  m_outcome = new wxChoice(this, -1);
  m_outcome->Append(_("(null)"));
  m_outcome->SetSelection(0);
  Gambit::Game efg = p_node->GetGame();
  for (int outc = 1; outc <= efg->NumOutcomes(); outc++) {
    Gambit::GameOutcome outcome = efg->GetOutcome(outc);
    std::string item = Gambit::lexical_cast<std::string>(outc) + ": " + outcome->GetLabel();
    if (item == "") {
      item = "Outcome" + Gambit::lexical_cast<std::string>(outc);
    }

    item += (" (" + 
	     Gambit::lexical_cast<std::string>(outcome->GetPayoff<std::string>(1)) + ", " +
	     Gambit::lexical_cast<std::string>(outcome->GetPayoff<std::string>(2)));
    if (efg->NumPlayers() > 2) {
      item += ", " + Gambit::lexical_cast<std::string>(outcome->GetPayoff<std::string>(3));
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

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

Gambit::GameInfoset dialogEditNode::GetInfoset(void) const
{
  if (m_infoset->GetSelection() == 0) {
    return 0;
  }
  else {
    return m_infosetList[m_infoset->GetSelection()];
  }
}
