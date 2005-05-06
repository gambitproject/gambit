//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing outcome payoffs
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
#include "game-document.h"
#include "dialog-outcome.h"

//======================================================================
//                     class gbtOutcomeDialog
//======================================================================

gbtOutcomeDialog::gbtOutcomeDialog(wxWindow *p_parent,
				   gbtGameDocument *p_doc,
				   const gbtGame &p_game,
				   const gbtGameOutcome &p_outcome)
  : wxDialog(p_parent, -1, _("Outcome payoffs"), wxDefaultPosition), 
    m_game(p_game), m_outcome(p_outcome)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_grid = new wxGrid(this, -1);
  m_grid->CreateGrid(p_game->NumPlayers() + 1, 1);
  m_grid->SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  m_grid->SetLabelFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  m_grid->SetRowLabelSize(150);
  m_grid->SetColLabelSize(0);
  m_grid->SetColSize(0, 125);

  m_grid->SetRowLabelValue(0, _("Label"));
  if (!p_outcome.IsNull()) {
    m_grid->SetCellValue(0, 0,
			 wxString(p_outcome->GetLabel().c_str(), 
				  *wxConvCurrent));
  }

  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    gbtGamePlayer player = p_game->GetPlayer(pl);
    m_grid->SetRowLabelValue(pl, 
			     wxString(player->GetLabel().c_str(),
				      *wxConvCurrent));
    m_grid->SetCellTextColour(pl, 0, p_doc->GetPlayerColor(pl));
    if (p_outcome.IsNull()) {
      m_grid->SetCellValue(pl, 0, wxT("0"));
    }
    else {
      m_grid->SetCellValue(pl, 0,
			   wxString(ToText(p_outcome->GetPayoff(player)).c_str(),
				    *wxConvCurrent));
    }
  }

  topSizer->Add(m_grid, 0, wxALL | wxALIGN_CENTER, 5);

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

gbtRational gbtOutcomeDialog::GetPayoff(int pl) const
{
  m_grid->SaveEditControlValue();

  gbtRational r;
  FromText((const char *) m_grid->GetCellValue(pl, 0).mb_str(), r);
  return r;
}
