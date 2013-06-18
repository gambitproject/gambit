//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlinsertmove.cc
// Implementation of dialog to insert a move into a tree
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
#include "dlinsertmove.h"

//=========================================================================
//                   gbtInsertMoveDialog: Member functions
//=========================================================================

gbtInsertMoveDialog::gbtInsertMoveDialog(wxWindow *p_parent, 
					 gbtGameDocument *p_doc)
  : wxDialog(p_parent, -1, _("Insert Move"), wxDefaultPosition), m_doc(p_doc)
{
  m_playerItem = new wxChoice(this, -1);
  m_playerItem->Append(_("Insert move for the chance player"));
  for (int pl = 1; pl <= m_doc->NumPlayers(); pl++) {
    wxString s = _("Insert move for ");
    Gambit::GamePlayer player = m_doc->GetGame()->GetPlayer(pl);
    if (player->GetLabel() != "") {
      s += wxString(player->GetLabel().c_str(), *wxConvCurrent);
    }
    else {
      s += wxString::Format(_("player %d"), pl);
    }
    m_playerItem->Append(s);
  }
  m_playerItem->Append(_("Insert move for a new player"));
  m_playerItem->SetSelection(1);
  Connect(m_playerItem->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	  wxCommandEventHandler(gbtInsertMoveDialog::OnPlayer));

  m_infosetItem = new wxChoice(this, -1);
  m_infosetItem->Append(_("at a new information set"));
  Gambit::GamePlayer player = m_doc->GetGame()->GetPlayer(1);
  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
    wxString s = _("at information set ");
    Gambit::GameInfoset infoset = player->GetInfoset(iset);
    if (infoset->GetLabel() != "") {
      s += wxString(infoset->GetLabel().c_str(), *wxConvCurrent);
    }
    else {
      s += wxString::Format(wxT("%d"), iset);
    }

    s += wxString::Format(wxT(" (%d action"), infoset->NumActions());
    if (infoset->NumActions() > 1) {
      s += wxT("s");
    }

    s += wxString::Format(wxT(", %d member node"), infoset->NumMembers());
    if (infoset->NumMembers() > 1) {
      s += wxT("s)");
    }
    else {
      s += wxT(")");
    }
    m_infosetItem->Append(s);
  }
  m_infosetItem->SetSelection(0);
  Connect(m_infosetItem->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	  wxCommandEventHandler(gbtInsertMoveDialog::OnInfoset));

  wxBoxSizer *actionSizer = new wxBoxSizer(wxHORIZONTAL);
  actionSizer->Add(new wxStaticText(this, wxID_STATIC, _("with")),
		   0, wxALL | wxALIGN_CENTER, 5);
  m_actions = new wxSpinCtrl(this, -1, _T("2"),
			     wxDefaultPosition, wxDefaultSize,
			     wxSP_ARROW_KEYS, 1, 10000, 2);
  m_actions->Enable(m_infosetItem->GetSelection() == 0);
  actionSizer->Add(m_actions, 0, wxALL, 5);
  actionSizer->Add(new wxStaticText(this, wxID_STATIC, _("actions")),
		   0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_playerItem, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5);
  topSizer->Add(m_infosetItem, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5);
  topSizer->Add(actionSizer, 0, wxALL | wxALIGN_CENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);
  
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
  CenterOnParent();
}

void gbtInsertMoveDialog::OnPlayer(wxCommandEvent &)
{
  int playerNumber = m_playerItem->GetSelection(); 

  Gambit::GamePlayer player;
  if (playerNumber == 0)
    player = m_doc->GetGame()->GetChance();
  else if (playerNumber <= m_doc->NumPlayers())
    player = m_doc->GetGame()->GetPlayer(playerNumber);

  m_infosetItem->Clear();
  m_infosetItem->Append(_("at a new information set"));

  if (!player) {
    m_infosetItem->SetSelection(0);
    m_actions->Enable(true);
    return;
  }

  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
    wxString s = _("at information set ");
    Gambit::GameInfoset infoset = player->GetInfoset(iset);
    if (infoset->GetLabel() != "") {
      s += wxString(infoset->GetLabel().c_str(), *wxConvCurrent);
    }
    else {
      s += wxString::Format(wxT("%d"), iset);
    }

    s += wxString::Format(wxT(" (%d action"), infoset->NumActions());
    if (infoset->NumActions() > 1) {
      s += wxT("s");
    }

    s += wxString::Format(wxT(", %d member node"), infoset->NumMembers());
    if (infoset->NumMembers() > 1) {
      s += wxT("s)");
    }
    else {
      s += wxT(")");
    }
    m_infosetItem->Append(s);
  }

  m_infosetItem->SetSelection(0);
  m_actions->Enable(true);
}

void gbtInsertMoveDialog::OnInfoset(wxCommandEvent &)
{
  int infosetNumber = m_infosetItem->GetSelection();

  if (infosetNumber > 0) {
    int playerNumber = m_playerItem->GetSelection();
    Gambit::GameInfoset infoset;
    if (playerNumber == 0)
      infoset = m_doc->GetGame()->GetChance()->GetInfoset(infosetNumber);
    else
      infoset = m_doc->GetGame()->GetPlayer(playerNumber)->GetInfoset(infosetNumber);
    m_actions->Enable(false);
    m_actions->SetValue(infoset->NumActions());
  }
  else {
    m_actions->Enable(true);
  }
}

Gambit::GamePlayer gbtInsertMoveDialog::GetPlayer(void) const
{
  int playerNumber = m_playerItem->GetSelection();

  if (playerNumber == 0) {
    return m_doc->GetGame()->GetChance();
  }
  else if (playerNumber <= m_doc->NumPlayers()) {
    return m_doc->GetGame()->GetPlayer(playerNumber);
  }
  else {
    Gambit::GamePlayer player = m_doc->GetGame()->NewPlayer();
    player->SetLabel("Player " + Gambit::lexical_cast<std::string>(m_doc->NumPlayers()));
    return player;
  }
}

Gambit::GameInfoset gbtInsertMoveDialog::GetInfoset(void) const
{
  if (m_playerItem->GetSelection() <= m_doc->NumPlayers()) {
    Gambit::GamePlayer player = GetPlayer();
    int infosetNumber = m_infosetItem->GetSelection();
    
    if (player && infosetNumber > 0) {
      return player->GetInfoset(infosetNumber);
    }
    else {
      return 0;
    }
  }
  else {
    return 0;
  }
}

int gbtInsertMoveDialog::GetActions(void) const
{
  return ((GetInfoset()) ? GetInfoset()->NumActions() : m_actions->GetValue());
}

