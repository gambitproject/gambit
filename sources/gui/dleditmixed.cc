//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to edit mixed strategies
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
#include "nash/mixedsol.h"
#include "dleditmixed.h"
#include "numberedit.h"

//-------------------------------------------------------------------------
//                class dialogEditMixed: Member functions
//-------------------------------------------------------------------------

const int idPLAYER_LIST = 2001;
const int idPROB_GRID = 2002;

BEGIN_EVENT_TABLE(dialogEditMixed, wxDialog)
  EVT_LISTBOX(idPLAYER_LIST, dialogEditMixed::OnSelChanged)
  EVT_BUTTON(wxID_OK, dialogEditMixed::OnOK)
END_EVENT_TABLE()

dialogEditMixed::dialogEditMixed(wxWindow *p_parent,
				 const MixedSolution &p_profile)
  : wxDialog(p_parent, -1, _("Mixed profile properties"),
	     wxDefaultPosition),
    m_profile(p_profile), m_selection(1)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *nameSizer = new wxBoxSizer(wxHORIZONTAL);
  nameSizer->Add(new wxStaticText(this, wxID_STATIC, _("Profile name")),
		 0, wxALL, 5);
  m_profileName = new wxTextCtrl(this, -1,
				 wxString::Format(wxT("%s"),
						  (char *) p_profile.GetLabel()));
  nameSizer->Add(m_profileName, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(nameSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);
  m_playerList = new wxListBox(this, idPLAYER_LIST);
  for (int pl = 1; pl <= m_profile.GetGame().NumPlayers(); pl++) {
    m_playerList->Append(wxString::Format(wxT("%s"),
					  (char *) (ToText(pl) + ": " +
						    m_profile.GetGame().GetPlayer(pl)->GetLabel())));
  }
  m_playerList->SetSelection(0);
  editSizer->Add(m_playerList, 0, wxALL, 5);

  gbtNfgPlayer firstPlayer = m_profile.GetGame().GetPlayer(1);
  m_probGrid = new wxGrid(this, idPROB_GRID,
			  wxDefaultPosition, wxDefaultSize);
  m_probGrid->CreateGrid(firstPlayer->NumStrategies(), 1);
  m_probGrid->SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  m_probGrid->SetDefaultEditor(new gbtNumberEditor);
  m_probGrid->SetLabelValue(wxHORIZONTAL, _("Probability"), 0);
  for (int st = 1; st <= firstPlayer->NumStrategies(); st++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      wxString::Format(wxT("%s"),
					       (char *) firstPlayer->GetStrategy(st).GetLabel()),
			      st - 1);
    m_probGrid->SetCellValue(wxString::Format(wxT("%s"),
					      (char *) ToText(p_profile(firstPlayer->GetStrategy(st)))),
			     st - 1, 0);
    if (st % 2 == 0) {
      m_probGrid->SetCellBackgroundColour(st - 1, 0, wxColour(200, 200, 200));
    }
    else {
      m_probGrid->SetCellBackgroundColour(st - 1, 0, wxColour(225, 225, 225));
    }
  }
  m_probGrid->SetMargins(0, 0);
  m_probGrid->SetSize(wxSize(m_probGrid->GetRowLabelSize() + 
			     m_probGrid->GetColSize(0), 200));
  editSizer->Add(m_probGrid, 0, wxALL, 5);
  topSizer->Add(editSizer, 0, wxEXPAND | wxALL, 5);

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

dialogEditMixed::~dialogEditMixed()
{ }

void dialogEditMixed::OnSelChanged(wxCommandEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  gbtNfgPlayer oldPlayer = m_profile.GetGame().GetPlayer(m_selection);

  for (int st = 1; st <= oldPlayer->NumStrategies(); st++) {
    m_profile.SetStrategyProb(oldPlayer->GetStrategy(st),
			      ToNumber(gbtText(m_probGrid->GetCellValue(st - 1, 0).mb_str())));
  }

  gbtNfgPlayer player = m_profile.GetGame().GetPlayer(p_event.GetSelection() + 1);

  if (oldPlayer->NumStrategies() > player->NumStrategies()) {
    m_probGrid->DeleteRows(0,
			   oldPlayer->NumStrategies() - player->NumStrategies());
  }
  else if (oldPlayer->NumStrategies() < player->NumStrategies()) {
    m_probGrid->InsertRows(0,
			   player->NumStrategies() - oldPlayer->NumStrategies());
  }

  for (int st = 1; st <= player->NumStrategies(); st++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      wxString::Format(wxT("%s"),
					       (char *) player->GetStrategy(st).GetLabel()),
			      st - 1);
    m_probGrid->SetCellValue(wxString::Format(wxT("%s"),
					      (char *) ToText(m_profile(player->GetStrategy(st)))),
			     st - 1, 0);
    if (st % 2 == 0) {
      m_probGrid->SetCellBackgroundColour(st - 1, 0, wxColour(200, 200, 200));
    }
    else {
      m_probGrid->SetCellBackgroundColour(st - 1, 0, wxColour(225, 225, 225));
    }
  }

  m_selection = p_event.GetSelection() + 1;
}

void dialogEditMixed::OnOK(wxCommandEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  gbtNfgPlayer player = m_profile.GetGame().GetPlayer(m_selection);

  for (int st = 1; st <= player->NumStrategies(); st++) {
    m_profile.SetStrategyProb(player->GetStrategy(st),
			      ToNumber(gbtText(m_probGrid->GetCellValue(st - 1, 0).mb_str())));
  }

  p_event.Skip();
}

const MixedSolution &dialogEditMixed::GetProfile(void) const
{
  m_profile.SetLabel(gbtText(m_profileName->GetLabel().mb_str()));
  return m_profile;
}
