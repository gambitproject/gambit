//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a contingency
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
#include "dleditcont.h"

//======================================================================
//                   class dialogEditContingency
//======================================================================

dialogEditContingency::dialogEditContingency(wxWindow *p_parent,
					     const gbtNfgGame &p_nfg,
					     const gArray<int> &p_cont)
  : wxDialog(p_parent, -1, "Contingency properties")
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *contSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, "Contingency"),
			 wxVERTICAL);
  for (int pl = 1; pl <= p_cont.Length(); pl++) {
    gbtNfgPlayer player = p_nfg.GetPlayer(pl);
    wxString text;
    if (player.GetLabel() != "") {
      text += wxString::Format("%s: ", (const char *) player.GetLabel());
    }
    else {
      text += wxString::Format("Player %d: ", pl);
    }

    if (player.GetStrategy(p_cont[pl]).GetLabel() != "") {
      text += (const char *) player.GetStrategy(p_cont[pl]).GetLabel();
    }
    else {
      text += wxString::Format("Strategy %d", p_cont[pl]);
    }

    contSizer->Add(new wxStaticText(this, wxID_STATIC, text), 0, wxALL, 5);
  }

  topSizer->Add(contSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *outcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  outcomeSizer->Add(new wxStaticText(this, wxID_STATIC, "Outcome"),
		    0, wxALL | wxCENTER, 5);
  m_outcome = new wxChoice(this, -1);
  m_outcome->Append("(null)");
  m_outcome->SetSelection(0);
  for (int outc = 1; outc <= p_nfg.NumOutcomes(); outc++) {
    gbtNfgOutcome outcome = p_nfg.GetOutcomeId(outc);
    gText item = ToText(outc) + ": " + outcome.GetLabel();
    if (item == "") {
      item = "Outcome" + ToText(outc);
    }

    item += (" (" + ToText(outcome.GetPayoff(p_nfg.GetPlayer(1))) + ", " +
	     ToText(outcome.GetPayoff(p_nfg.GetPlayer(2))));
    if (p_nfg.NumPlayers() > 2) {
      item += ", " + ToText(outcome.GetPayoff(p_nfg.GetPlayer(3)));
      if (p_nfg.NumPlayers() > 3) {
	item += ",...)";
      }
      else {
	item += ")";
      }
    }
    else {
      item += ")";
    }

    m_outcome->Append((const char *) item);
    if (p_nfg.GetOutcome(p_cont) == outcome) {
      m_outcome->SetSelection(outc);
    }
  }
  outcomeSizer->Add(m_outcome, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(outcomeSizer, 0, wxALL | wxEXPAND, 5);

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
