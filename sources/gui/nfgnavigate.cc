//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of normal form navigation window
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
#endif // WX_PRECOMP
#include "nfgnavigate.h"

const int idSTRATEGY_CHOICE = 2001;
const int idROWPLAYER_CHOICE = 2002;
const int idCOLPLAYER_CHOICE = 2003;

BEGIN_EVENT_TABLE(NfgNavigateWindow, wxPanel)
  EVT_CHOICE(idSTRATEGY_CHOICE, NfgNavigateWindow::OnStrategyChange)
  EVT_CHOICE(idROWPLAYER_CHOICE, NfgNavigateWindow::OnRowPlayerChange)
  EVT_CHOICE(idCOLPLAYER_CHOICE, NfgNavigateWindow::OnColPlayerChange)
END_EVENT_TABLE()

NfgNavigateWindow::NfgNavigateWindow(gbtGameDocument *p_doc,
				     wxWindow *p_parent)
  : wxPanel(p_parent, -1), gbtGameView(p_doc),
    m_rowPlayer(1), m_colPlayer(2)
{
  gbtNfgGame nfg = *m_doc->m_nfg;

  wxStaticBoxSizer *playerViewSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "View players"),
			 wxVERTICAL);

  wxBoxSizer *rowChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
  rowChoiceSizer->Add(new wxStaticText(this, -1, "Row player"),
		      1, wxALIGN_LEFT | wxRIGHT, 5);
  m_rowChoice = new wxChoice(this, idROWPLAYER_CHOICE);
  rowChoiceSizer->Add(m_rowChoice, 0, wxALL, 0);

  wxBoxSizer *colChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
  colChoiceSizer->Add(new wxStaticText(this, -1, "Column player"),
		      1, wxALIGN_LEFT | wxRIGHT, 5);
  m_colChoice = new wxChoice(this, idCOLPLAYER_CHOICE);
  colChoiceSizer->Add(m_colChoice, 0, wxALL, 0);

  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    wxString playerName = (char *) (ToText(pl) + ": " +
				    nfg.GetPlayer(pl).GetLabel());
    m_rowChoice->Append(playerName);
    m_colChoice->Append(playerName);
  }

  m_rowChoice->SetSelection(0);
  m_colChoice->SetSelection(1);

  playerViewSizer->Add(rowChoiceSizer, 0, wxALL | wxEXPAND, 5);
  playerViewSizer->Add(colChoiceSizer, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *contViewSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Current contingency"),
			 wxVERTICAL);

  m_playerNames = new wxStaticText *[nfg.NumPlayers()];
  m_stratProfile = new wxChoice *[nfg.NumPlayers()];
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    m_stratProfile[pl-1] = new wxChoice(this, idSTRATEGY_CHOICE);
    
    gbtNfgPlayer player = nfg.GetPlayer(pl);
    for (int st = 1; st <= player.NumStrategies(); st++) {
      m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					     player.GetStrategy(st).GetLabel()));
    }
    m_stratProfile[pl-1]->SetSelection(0);

    wxBoxSizer *stratSizer = new wxBoxSizer(wxHORIZONTAL);
    if (player.GetLabel() != "") {
      m_playerNames[pl-1] = new wxStaticText(this, wxID_STATIC,
					     (char *) player.GetLabel());
    }
    else {
      m_playerNames[pl-1] = new wxStaticText(this, wxID_STATIC,
					     wxString::Format("Player %d",
							      pl));
    }
    stratSizer->Add(m_playerNames[pl-1], 1, wxALIGN_LEFT | wxRIGHT, 5);
    stratSizer->Add(m_stratProfile[pl-1], 0, wxALL, 0);
    contViewSizer->Add(stratSizer, 0, wxALL | wxEXPAND, 5);
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(playerViewSizer, 0, wxALL | wxEXPAND, 10);
  topSizer->Add(contViewSizer, 0, wxALL | wxEXPAND, 10);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  Show(true);
}

NfgNavigateWindow::~NfgNavigateWindow()
{
  delete [] m_playerNames;
  delete [] m_stratProfile;
}

void NfgNavigateWindow::OnUpdate(gbtGameView *)
{
  gbtNfgSupport *support = m_doc->GetNfgSupport();

  for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
    m_stratProfile[pl-1]->Clear();
    gbtNfgPlayer player = m_doc->GetNfg().GetPlayer(pl);
    for (int st = 1; st <= player.NumStrategies(); st++) {
      if (support->Contains(player.GetStrategy(st))) {
	m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					       player.GetStrategy(st).GetLabel()));
      }
    }
    m_stratProfile[pl-1]->SetSelection(m_doc->GetContingency()[pl] - 1);
  }
  m_rowChoice->SetSelection(m_doc->GetRowPlayer() - 1);
  m_colChoice->SetSelection(m_doc->GetColPlayer() - 1);
}

void NfgNavigateWindow::OnStrategyChange(wxCommandEvent &)
{
  gArray<int> contingency(m_doc->GetNfg().NumPlayers());
  for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
    contingency[pl] = m_stratProfile[pl-1]->GetSelection() + 1;
  }
  m_doc->SetContingency(contingency);
}

void NfgNavigateWindow::OnRowPlayerChange(wxCommandEvent &)
{
  int oldRowPlayer = m_doc->GetRowPlayer();
  int newRowPlayer = m_rowChoice->GetSelection() + 1;

  if (newRowPlayer == oldRowPlayer) {
    return;
  }

  m_doc->SetRowPlayer(newRowPlayer);
}

void NfgNavigateWindow::OnColPlayerChange(wxCommandEvent &)
{
  int oldColPlayer = m_doc->GetColPlayer();
  int newColPlayer = m_colChoice->GetSelection() + 1;

  if (newColPlayer == oldColPlayer) {
    return;
  }

  m_doc->SetColPlayer(newColPlayer);
}

