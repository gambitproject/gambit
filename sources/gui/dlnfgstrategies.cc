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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "game/game.h"
#include "game/nfstrat.h"
#include "dlnfgstrategies.h"

//=========================================================================
//                    dialogStrategies: Member functions
//=========================================================================

const int GBT_PLAYER_LIST = 2000;
const int GBT_EDIT_GRID = 2001;

BEGIN_EVENT_TABLE(dialogStrategies, wxDialog)
  EVT_LISTBOX(GBT_PLAYER_LIST, dialogStrategies::OnSelChanged)
  EVT_BUTTON(wxID_OK, dialogStrategies::OnOK)
END_EVENT_TABLE()

dialogStrategies::dialogStrategies(wxWindow *p_parent, const gbtGame &p_nfg)
  : wxDialog(p_parent, -1, _("Strategy Labels"), wxDefaultPosition), 
    m_nfg(p_nfg), m_selection(1)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxVERTICAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, _("Players:")),
		   0, wxALL | wxCENTER, 5);
  m_playerList = new wxListBox(this, GBT_PLAYER_LIST);
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    gbtGamePlayer player = m_nfg->GetPlayer(pl);
    m_playerList->Append(wxString::Format(wxT("%s"),
					  (char *) (ToText(pl) + ": " +
						    player->GetLabel())));
    m_strategyNames.Append(gbtArray<gbtText>(player->NumStrategies()));
    for (int st = 1; st <= player->NumStrategies(); st++) {
      m_strategyNames[pl][st] = player->GetStrategy(st)->GetLabel();
    }
  }
  m_playerList->SetSelection(0);
  playerSizer->Add(m_playerList, 0, wxALL, 5);
  editSizer->Add(playerSizer, 0, wxALL, 5);

  gbtGamePlayer firstPlayer = m_nfg->GetPlayer(1);
  m_editGrid = new wxGrid(this, GBT_EDIT_GRID,
			  wxDefaultPosition, wxDefaultSize);
  m_editGrid->CreateGrid(firstPlayer->NumStrategies(), 1);
  m_editGrid->SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  m_editGrid->SetLabelValue(wxHORIZONTAL, _("Label"), 0);
  for (int st = 1; st <= firstPlayer->NumStrategies(); st++) {
    m_editGrid->SetLabelValue(wxVERTICAL, wxString::Format(wxT("%d"), st), 
			      st - 1);
    m_editGrid->SetCellValue(wxString::Format(wxT("%s"),
					      (char *) firstPlayer->GetStrategy(st)->GetLabel()),
			     st - 1, 0);
    if (st % 2 == 0) {
      m_editGrid->SetCellBackgroundColour(st - 1, 0, wxColour(200, 200, 200));
    }
    else {
      m_editGrid->SetCellBackgroundColour(st - 1, 0, wxColour(225, 225, 225));
    }
  }
  m_editGrid->SetMargins(0, 0);
  m_editGrid->SetSize(wxSize(m_editGrid->GetRowLabelSize() + 
			     m_editGrid->GetColSize(0), 200));
  editSizer->Add(m_editGrid, 0, wxALL, 5);
  topSizer->Add(editSizer, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}

void dialogStrategies::OnSelChanged(wxCommandEvent &p_event)
{
  if (m_editGrid->IsCellEditControlEnabled()) {
    m_editGrid->SaveEditControlValue();
    m_editGrid->HideCellEditControl();
  }

  gbtGamePlayer oldPlayer = m_nfg->GetPlayer(m_selection);

  for (int st = 1; st <= oldPlayer->NumStrategies(); st++) {
    m_strategyNames[m_selection][st] = gbtText(m_editGrid->GetCellValue(st - 1, 0).mb_str());
  }

  gbtGamePlayer player = m_nfg->GetPlayer(p_event.GetSelection() + 1);

  if (oldPlayer->NumStrategies() > player->NumStrategies()) {
    m_editGrid->DeleteRows(0,
			   oldPlayer->NumStrategies() - player->NumStrategies());
  }
  else if (oldPlayer->NumStrategies() < player->NumStrategies()) {
    m_editGrid->InsertRows(0,
			   player->NumStrategies() - oldPlayer->NumStrategies());
  }

  m_selection = p_event.GetSelection() + 1;

  for (int st = 1; st <= player->NumStrategies(); st++) {
    m_editGrid->SetLabelValue(wxVERTICAL, wxString::Format(wxT("%d"), st),
			      st - 1);
    m_editGrid->SetCellValue(wxString::Format(wxT("%s"),
					      (char *) m_strategyNames[m_selection][st]),
			     st - 1, 0);
    if (st % 2 == 0) {
      m_editGrid->SetCellBackgroundColour(st - 1, 0, wxColour(200, 200, 200));
    }
    else {
      m_editGrid->SetCellBackgroundColour(st - 1, 0, wxColour(225, 225, 225));
    }
  }
 
  m_editGrid->AdjustScrollbars();
}

void dialogStrategies::OnOK(wxCommandEvent &p_event)
{
  if (m_editGrid->IsCellEditControlEnabled()) {
    m_editGrid->SaveEditControlValue();
    m_editGrid->HideCellEditControl();
  }

  gbtGamePlayer player = m_nfg->GetPlayer(m_selection);

  for (int st = 1; st <= player->NumStrategies(); st++) {
    m_strategyNames[m_selection][st] = (char *) m_editGrid->GetCellValue(st - 1, 0).mb_str();
  }

  p_event.Skip();
}


class gbtCmdEditStrategies : public gbtGameCommand {
private:
  gbtBlock<gbtArray<gbtText> > m_strategies;

public:
  gbtCmdEditStrategies(const gbtBlock<gbtArray<gbtText> > &p_strategies)
    : m_strategies(p_strategies) { }
  virtual ~gbtCmdEditStrategies() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdEditStrategies::Do(gbtGameDocument *p_doc)
{
  for (int pl = 1; pl <= p_doc->GetGame()->NumPlayers(); pl++) {
    gbtGamePlayer player = p_doc->GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      player->GetStrategy(st)->SetLabel(m_strategies[pl][st]);
    }
  }
}

gbtGameCommand *dialogStrategies::GetCommand(void) const
{
  return new gbtCmdEditStrategies(m_strategyNames);
}
