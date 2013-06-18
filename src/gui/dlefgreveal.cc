//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlefgreveal.cc
// Dialog for revealing actions to players
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
#include "dlefgreveal.h"


//=========================================================================
//                  gbtRevealMoveDialog: Member functions
//=========================================================================

gbtRevealMoveDialog::gbtRevealMoveDialog(wxWindow *p_parent,
					 gbtGameDocument *p_doc)
  : wxDialog(p_parent, -1, _("Reveal this move to players"), 
	     wxDefaultPosition), 
    m_doc(p_doc)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *playerBox =
    new wxStaticBoxSizer(wxHORIZONTAL, this, 
			 _("Reveal the move to players"));

  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);	

  for (int pl = 1; pl <= m_doc->NumPlayers(); pl++) {
    Gambit::GamePlayer player = m_doc->GetGame()->GetPlayer(pl);
    if (player->GetLabel() != "") {
      m_players.Append(new wxCheckBox(this, -1, 
				      wxString(player->GetLabel().c_str(),
					       *wxConvCurrent)));
    }
    else {
      m_players.Append(new wxCheckBox(this, -1, 
				      wxString::Format(_T("Player %d"), pl)));
    }
    m_players[pl]->SetValue(1);
    m_players[pl]->SetForegroundColour(m_doc->GetStyle().GetPlayerColor(pl));
    boxSizer->Add(m_players[pl], 1, wxALL | wxEXPAND, 0);
  }
  playerBox->Add(boxSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerBox, 1, wxALL | wxEXPAND, 5);

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

Gambit::Array<Gambit::GamePlayer> gbtRevealMoveDialog::GetPlayers(void) const
{
  Gambit::Array<Gambit::GamePlayer> players;

  for (int pl = 1; pl <= m_doc->NumPlayers(); pl++) {
    if (m_players[pl]->GetValue()) {
      players.Append(m_doc->GetGame()->GetPlayer(pl));
    }
  }

  return players;
}
