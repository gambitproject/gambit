//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to create a new game
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
#include "dlnewgame.h"

//========================================================================
//                        class dialogNewGame
//========================================================================

const int idRADIOBOX_GAMETYPE = 2000;
const int idSPINCTRL_NUMPLAYERS = 2001;
const int idLISTBOX_STRATEGIES = 2002;
const int idSPINCTRL_NUMSTRATEGIES = 2003;

BEGIN_EVENT_TABLE(dialogNewGame, wxDialog)
  EVT_RADIOBOX(idRADIOBOX_GAMETYPE, dialogNewGame::OnGameType)
  EVT_SPINCTRL(idSPINCTRL_NUMPLAYERS, dialogNewGame::OnNumPlayers)
  EVT_LISTBOX(idLISTBOX_STRATEGIES, dialogNewGame::OnStrategy)
  EVT_SPINCTRL(idSPINCTRL_NUMSTRATEGIES, dialogNewGame::OnNumStrategies)
END_EVENT_TABLE()

dialogNewGame::dialogNewGame(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "Create a new game")
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString typeChoices[] = { "Extensive form", "Normal form" };
  m_gameType = new wxRadioBox(this, idRADIOBOX_GAMETYPE, "Representation",
			      wxDefaultPosition, wxDefaultSize,
			      2, typeChoices, 1, wxRA_SPECIFY_ROWS);
  m_gameType->SetSelection(0);
  topSizer->Add(m_gameType, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *playersSizer = new wxBoxSizer(wxHORIZONTAL);
  playersSizer->Add(new wxStaticText(this, wxID_STATIC, "Number of players"),
		    0, wxALL, 5);
  m_numPlayers = new wxSpinCtrl(this, idSPINCTRL_NUMPLAYERS, "2",
			     wxDefaultPosition, wxDefaultSize,
			     wxSP_ARROW_KEYS, 2, 1000);
  playersSizer->Add(m_numPlayers, 0, wxALL, 5);
  topSizer->Add(playersSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *nfgBox = new wxStaticBox(this, wxID_STATIC,
					"Normal form parameters"); 
  wxStaticBoxSizer *nfgTopSizer = new wxStaticBoxSizer(nfgBox, wxVERTICAL);

  wxBoxSizer *nfgSizer = new wxBoxSizer(wxHORIZONTAL);
  m_strategyList = new wxListBox(this, idLISTBOX_STRATEGIES);
  m_strategyList->Append("Player1: 2 strategies");
  m_strategyList->Append("Player2: 2 strategies");
  m_strategies.Append(2);
  m_strategies.Append(2);
  m_strategyList->Enable(false);
  nfgSizer->Add(m_strategyList, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *strategiesSizer = new wxBoxSizer(wxVERTICAL);
  strategiesSizer->Add(new wxStaticText(this, wxID_STATIC, "Strategies"),
		       0, wxALL | wxCENTER, 5);
  m_numStrategies = new wxSpinCtrl(this, idSPINCTRL_NUMSTRATEGIES, "2",
				   wxDefaultPosition, wxDefaultSize,
				   wxSP_ARROW_KEYS, 1, 10000);
  m_numStrategies->Enable(false);
  strategiesSizer->Add(m_numStrategies, 0, wxALL | wxCENTER, 5);
  nfgSizer->Add(strategiesSizer, 0, wxALL, 5);
  nfgTopSizer->Add(nfgSizer, 1, wxALL | wxEXPAND, 5);

  m_createOutcomes = new wxCheckBox(this, -1, "Automatically create outcomes");
  m_createOutcomes->SetValue(true);
  m_createOutcomes->Enable(false);
  nfgTopSizer->Add(m_createOutcomes, 0, wxALL | wxCENTER, 5);
  topSizer->Add(nfgTopSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

void dialogNewGame::OnGameType(wxCommandEvent &)
{
  m_strategyList->Enable(m_gameType->GetSelection() == 1);
  m_numStrategies->Enable(m_gameType->GetSelection() == 1);
  m_createOutcomes->Enable(m_gameType->GetSelection() == 1);
}

void dialogNewGame::OnNumPlayers(wxSpinEvent &)
{
  int numPlayers = m_numPlayers->GetValue();

  while (m_strategyList->GetCount() > numPlayers) {
    m_strategyList->Delete(m_strategyList->GetCount() - 1);
    m_strategies.Remove(m_strategies.Length());
  }
  while (m_strategyList->GetCount() < numPlayers) {
    m_strategyList->Append(wxString::Format("Player%d: 2 strategies",
					    m_strategyList->GetCount() + 1));
    m_strategies.Append(2);
  }
}

void dialogNewGame::OnStrategy(wxCommandEvent &)
{
  m_numStrategies->SetValue(m_strategies[m_strategyList->GetSelection() + 1]);
}

void dialogNewGame::OnNumStrategies(wxSpinEvent &)
{
  int numStrategies = m_numStrategies->GetValue();
  int player = m_strategyList->GetSelection() + 1;
  m_strategyList->SetString(m_strategyList->GetSelection(),
			    wxString::Format("Player%d: %d strategies",
					     player, numStrategies));
  m_strategies[player] = numStrategies;
}
