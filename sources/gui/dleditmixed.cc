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
  : wxDialog(p_parent, -1, "Mixed profile properties"),
    m_profile(p_profile), m_selection(1)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *nameSizer = new wxBoxSizer(wxHORIZONTAL);
  nameSizer->Add(new wxStaticText(this, wxID_STATIC, "Profile name"),
		 0, wxALL, 5);
  m_profileName = new wxTextCtrl(this, -1, (char *) p_profile.GetName());
  nameSizer->Add(m_profileName, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(nameSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);
  m_playerList = new wxListBox(this, idPLAYER_LIST);
  for (int pl = 1; pl <= m_profile.Game().NumPlayers(); pl++) {
    m_playerList->Append((char *) (ToText(pl) + ": " +
				   m_profile.Game().Players()[pl]->GetName()));
  }
  m_playerList->SetSelection(0);
  editSizer->Add(m_playerList, 0, wxALL, 5);

  NFPlayer *firstPlayer = m_profile.Game().Players()[1];
  m_probGrid = new wxGrid(this, idPROB_GRID,
			  wxDefaultPosition, wxSize(200, 200));
  m_probGrid->CreateGrid(firstPlayer->NumStrats(), 1);
  m_probGrid->SetLabelValue(wxHORIZONTAL, "Probability", 0);
  for (int st = 1; st <= firstPlayer->NumStrats(); st++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      (char *) firstPlayer->Strategies()[st]->Name(),
			      st - 1);
    m_probGrid->SetCellValue((char *) ToText(p_profile(firstPlayer->Strategies()[st])),
			     st - 1, 0);
  }
  m_probGrid->UpdateDimensions();
  m_probGrid->Refresh();
  editSizer->Add(m_probGrid, 0, wxALL, 5);
  topSizer->Add(editSizer, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
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

  NFPlayer *oldPlayer = m_profile.Game().Players()[m_selection];

  for (int st = 1; st <= oldPlayer->NumStrats(); st++) {
    m_profile.Set(oldPlayer->Strategies()[st],
		  ToNumber(m_probGrid->GetCellValue(st - 1, 0).c_str()));
  }

  NFPlayer *player = m_profile.Game().Players()[p_event.GetSelection() + 1];

  if (oldPlayer->NumStrats() > player->NumStrats()) {
    m_probGrid->DeleteRows(0,
			   oldPlayer->NumStrats() - player->NumStrats());
  }
  else if (oldPlayer->NumStrats() < player->NumStrats()) {
    m_probGrid->InsertRows(0,
			   player->NumStrats() - oldPlayer->NumStrats());
  }

  for (int st = 1; st <= player->NumStrats(); st++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      (char *) player->Strategies()[st]->Name(),
			      st - 1);
    m_probGrid->SetCellValue((char *) ToText(m_profile(player->Strategies()[st])),
			     st - 1, 0);
  }

  m_selection = p_event.GetSelection() + 1;
}

void dialogEditMixed::OnOK(wxCommandEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  NFPlayer *player = m_profile.Game().Players()[m_selection];

  for (int st = 1; st <= player->NumStrats(); st++) {
    m_profile.Set(player->Strategies()[st],
		  ToNumber(m_probGrid->GetCellValue(st - 1, 0).c_str()));
  }

  p_event.Skip();
}

const MixedSolution &dialogEditMixed::GetProfile(void) const
{
  m_profile.SetName(m_profileName->GetValue().c_str());
  return m_profile;
}
