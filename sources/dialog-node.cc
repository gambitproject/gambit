//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a node
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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
#include <libgambit/libgambit.h>
#include "dialog-node.h"

//======================================================================
//                      class gbtNodeDialog
//======================================================================

gbtNodeDialog::gbtNodeDialog(wxWindow *p_parent, 
			     gbtGame p_game,
			     gbtGameNode p_node)
  : wxDialog(p_parent, -1, _("Node properties"), wxDefaultPosition), 
    m_node(p_node)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Node label")),
		  0, wxALL | wxCENTER, 5);
  m_nodeLabel = new wxTextCtrl(this, -1, 
			       wxString::Format(_T("%s"), 
						m_node->GetLabel().c_str()));
  labelSizer->Add(m_nodeLabel, 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *infosetSizer = new wxBoxSizer(wxHORIZONTAL);
  infosetSizer->Add(new wxStaticText(this, wxID_STATIC, _("Information set")),
		    0, wxALL | wxALIGN_CENTER, 5);
  m_infoset = new wxChoice(this, -1);
  if (p_node->NumChildren() > 0) {
    m_infoset->Append(_("New information set"));
    gbtBlock<gbtGameInfoset> infosets;
    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      gbtGamePlayer player = p_game->GetPlayer(pl);
      for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	infosets.Append(player->GetInfoset(iset));
      }
    }

    int selection = 0;
    for (int iset = 1; iset <= infosets.Length(); iset++) {
      if (!infosets[iset]->IsChanceInfoset() &&
	  infosets[iset]->NumActions() == p_node->NumChildren()) {
	m_infosetList.Append(infosets[iset]);
	m_infoset->Append(wxString::Format(_("Player %d, Infoset %d"),
			  infosets[iset]->GetPlayer()->GetId(),
			  infosets[iset]->GetId()));
	if (infosets[iset] == p_node->GetInfoset()) {
	  selection = m_infosetList.Length();
	}
      }
    }
    m_infoset->SetSelection(selection);
  }
  else {
    m_infoset->Append(_("(none)"));
    m_infoset->SetSelection(0);
    m_infoset->Enable(false);
  }
  infosetSizer->Add(m_infoset, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(infosetSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *outcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  outcomeSizer->Add(new wxStaticText(this, wxID_STATIC, _("Outcome")),
		    0, wxALL | wxALIGN_CENTER, 5);
  m_outcome = new wxChoice(this, -1);
  m_outcome->Append(_("(No outcome)"));
  m_outcome->SetSelection(0);

  for (int outc = 1; outc <= p_game->NumOutcomes(); outc++) {
    gbtGameOutcome outcome = p_game->GetOutcome(outc);
    wxString item = wxString::Format(wxT("%d: %s"),
				     outc,
				     outcome->GetLabel().c_str());
    if (item == "") {
      item = wxString::Format(wxT("Outcome%d"), outc);
    }

    /*
    item += wxString::Format(wxT("%s"),
			     ToText(outcome->GetPayoff(p_game->GetPlayer(1)))
	     ToText(efg.Payoff(outcome, efg.Players()[2])));
    if (efg.NumPlayers() > 2) {
      item += ", " + ToText(efg.Payoff(outcome, efg.Players()[3]));
      if (efg.NumPlayers() > 3) {
	item += ",...)";
      }
      else {
	item += ")";
      }
    }
    else {
      item += ")";
    }
    */

    m_outcome->Append(item);
    if (m_node->GetOutcome() == outcome) {
      m_outcome->SetSelection(outc);
    }
  }
  outcomeSizer->Add(m_outcome, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(outcomeSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

gbtGameInfoset gbtNodeDialog::GetInfoset(void) const
{
  if (m_infoset->GetSelection() == 0) {
    return 0;
  }
  else {
    return m_infosetList[m_infoset->GetSelection()];
  }
}
