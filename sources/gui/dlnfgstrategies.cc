//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to inspect and edit normal form strategies
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "game/nfg.h"
#include "game/nfstrat.h"
#include "dlnfgstrategies.h"

//=========================================================================
//                    dialogStrategies: Member functions
//=========================================================================

const int idCHOICE_PLAYER = 2000;
const int idLISTBOX_STRATEGIES = 2001;

BEGIN_EVENT_TABLE(dialogStrategies, wxDialog)
  EVT_CHOICE(idCHOICE_PLAYER, dialogStrategies::OnPlayerChanged)
  EVT_LISTBOX(idLISTBOX_STRATEGIES, dialogStrategies::OnStrategyChanged)
  EVT_BUTTON(wxID_OK, dialogStrategies::OnOK)
END_EVENT_TABLE()

dialogStrategies::dialogStrategies(wxWindow *p_parent, const gbtNfgGame &p_nfg)
  : wxDialog(p_parent, -1, _("Strategies"), wxDefaultPosition), 
    m_nfg(p_nfg), m_lastPlayer(0), m_lastStrategy(0)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Strategies for player")),
		   0, wxALL, 5);
  m_player = new wxChoice(this, idCHOICE_PLAYER);
  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    gbtNfgPlayer player = p_nfg.GetPlayer(pl);
    m_player->Append(wxString::Format(wxT("%d: %s"), pl,
				      (char *) player.GetLabel())); 
    m_strategyNames.Append(gbtArray<gbtText>(player.NumStrategies()));
    for (int st = 1; st <= player.NumStrategies(); st++) {
      m_strategyNames[pl][st] = player.GetStrategy(st).GetLabel();
    }
  } 
  m_player->SetSelection(0);
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *strategyBoxSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, _("Strategies")),
			 wxHORIZONTAL);
  m_strategyList = new wxListBox(this, idLISTBOX_STRATEGIES);
  for (int st = 1; st <= m_nfg.GetPlayer(1).NumStrategies(); st++) {
    m_strategyList->Append(wxString::Format(wxT("%d: %s"), st,
					    (char *) m_strategyNames[1][st]));
  }
  m_strategyList->SetSelection(0);
  strategyBoxSizer->Add(m_strategyList, 0, wxALL, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxVERTICAL);
  editSizer->Add(new wxStaticText(this, wxID_STATIC, _("Strategy name")),
		 0, wxALL | wxCENTER, 5);
  m_strategyName = new wxTextCtrl(this, -1, 
				  wxString::Format(wxT("%s"),
						   (char *) m_strategyNames[1][1]));
  editSizer->Add(m_strategyName, 0, wxALL | wxCENTER, 5);
  strategyBoxSizer->Add(editSizer, 0, wxALL, 5);

  topSizer->Add(strategyBoxSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, _("Help")), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);
  
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
  CenterOnParent();
}

void dialogStrategies::OnPlayerChanged(wxCommandEvent &)
{
  m_strategyNames[m_lastPlayer+1][m_lastStrategy+1] =
    m_strategyName->GetValue().mb_str();
  m_strategyList->Clear();
  int player = m_player->GetSelection() + 1;
  for (int st = 1; st <= m_strategyNames[player].Length(); st++) {
    m_strategyList->Append(wxString::Format(wxT("%d: %s"), st,
					    (char *) m_strategyNames[player][st]));
  }
  m_strategyList->SetSelection(0);
  m_strategyName->SetValue(wxString::Format(wxT("%s"),
					    (char *) m_strategyNames[player][1]));
  m_lastPlayer = m_player->GetSelection();
  m_lastStrategy = 0;
}

void dialogStrategies::OnStrategyChanged(wxCommandEvent &)
{
  int player = m_player->GetSelection() + 1;
  m_strategyNames[player][m_lastStrategy+1] =
    m_strategyName->GetValue().mb_str();
  m_strategyList->SetString(m_lastStrategy,
			    wxString::Format(wxT("%d: %s"), m_lastStrategy + 1,
					     m_strategyName->GetValue().c_str()));
  m_lastStrategy = m_strategyList->GetSelection();
  m_strategyName->SetValue(wxString::Format(wxT("%s"),
					    (char *) m_strategyNames[player][m_lastStrategy+1]));
}

void dialogStrategies::OnOK(wxCommandEvent &p_event)
{
  // Copy any edited data into the blocks
  int player = m_player->GetSelection() + 1;
  m_strategyNames[player][m_lastStrategy+1] =
    m_strategyName->GetValue().mb_str();
  // Go on with usual processing
  p_event.Skip();
}
