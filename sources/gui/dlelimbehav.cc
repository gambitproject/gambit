//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for parameters to eliminate dominated actions
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
#include "base/base.h"
#include "dlelimbehav.h"

//=========================================================================
//                  class dialogElimBehav: Member functions
//=========================================================================

dialogElimBehav::dialogElimBehav(wxWindow *p_parent, 
				 const gArray<gText> &p_players)
  : wxDialog(p_parent, -1, "Computing Dominated Actions"),
    m_numPlayers(p_players.Length())
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *optionSizer = new wxBoxSizer(wxHORIZONTAL);
  wxString domTypeList[2] = { "Weak", "Strong" };
  m_domTypeBox = new wxRadioBox(this, -1, "Type",
				wxDefaultPosition, wxDefaultSize,
				2, domTypeList, 1, wxRA_SPECIFY_COLS);
  m_domTypeBox->SetSelection(0);
  optionSizer->Add(m_domTypeBox, 0, wxALL, 5);
  
  wxString domConditionalList[2] = { "Conditional", "Unconditional" };
  m_domConditionalBox = new wxRadioBox(this, -1, "Conditional",
				       wxDefaultPosition, wxDefaultSize,
				       2, domConditionalList,
				       1, wxRA_SPECIFY_COLS);
  m_domConditionalBox->SetSelection(0);
  optionSizer->Add(m_domConditionalBox, 0, wxALL, 5);

  wxString domDepthList[2] = { "One level", "Iteratively" };
  m_domDepthBox = new wxRadioBox(this, -1, "Depth",
				 wxDefaultPosition, wxDefaultSize,
				 2, domDepthList, 1, wxRA_SPECIFY_COLS);
  optionSizer->Add(m_domDepthBox, 0, wxALL, 5);
  topSizer->Add(optionSizer, 0, wxALL, 5);

  wxStaticBoxSizer *playerBox =
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, "Players"),
			 wxHORIZONTAL);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(wxVERTICAL,
						   (p_players.Length() <= 2) ?
						   2 : 3);
  m_players = new wxCheckBox *[p_players.Length()];
  for (int pl = 1; pl <= p_players.Length(); pl++) {
    if (p_players[pl] != "") {
      m_players[pl-1] = new wxCheckBox(this, -1, (char *) p_players[pl]);
    }
    else {
      m_players[pl-1] = new wxCheckBox(this, -1, 
				       wxString::Format("Player %d", pl));
    }
    m_players[pl-1]->SetValue(1);
    gridSizer->Add(m_players[pl-1], 1, wxALL | wxEXPAND, 0);
  }
  playerBox->Add(gridSizer, 1, wxALL, 5);
  topSizer->Add(playerBox, 1, wxALL | wxCENTER, 5);

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

dialogElimBehav::~dialogElimBehav()
{
  delete [] m_players;
}

gArray<int> dialogElimBehav::Players(void) const
{
  gBlock<int> players;
  for (int pl = 1; pl <= m_numPlayers; pl++) {
    if (m_players[pl-1]->GetValue() == 1) {
      players.Append(pl);
    }
  }
  return players;
}
