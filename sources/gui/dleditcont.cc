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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include "game/game.h"
#include "game/nfgcont.h"
#include "dleditcont.h"

//======================================================================
//                   class dialogEditContingency
//======================================================================

dialogEditContingency::dialogEditContingency(wxWindow *p_parent,
					     const gbtGame &p_nfg,
					     const gbtArray<int> &p_cont)
  : wxDialog(p_parent, -1, _("Contingency properties"), wxDefaultPosition)
{
  SetAutoLayout(true);

  gbtNfgContingency profile = p_nfg->NewContingency();
  for (int pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
    profile->SetStrategy(p_nfg->GetPlayer(pl)->GetStrategy(p_cont[pl]));
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *contSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, _("Contingency")),
			 wxVERTICAL);
  for (int pl = 1; pl <= p_cont.Length(); pl++) {
    gbtGamePlayer player = p_nfg->GetPlayer(pl);
    wxString text;
    if (player->GetLabel() != "") {
      text += wxString::Format(wxT("%s: "), (const char *) player->GetLabel());
    }
    else {
      text += wxString::Format(wxT("Player %d: "), pl);
    }

    if (player->GetStrategy(p_cont[pl])->GetLabel() != "") {
      text += wxString::Format(wxT("%s"),
			       (const char *) player->GetStrategy(p_cont[pl])->GetLabel());
    }
    else {
      text += wxString::Format(wxT("Strategy %d"), p_cont[pl]);
    }

    contSizer->Add(new wxStaticText(this, wxID_STATIC, text), 0, wxALL, 5);
  }

  topSizer->Add(contSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *outcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  outcomeSizer->Add(new wxStaticText(this, wxID_STATIC, _("Outcome")),
		    0, wxALL | wxCENTER, 5);
  m_outcome = new wxChoice(this, -1);
  m_outcome->Append(_("(null)"));
  m_outcome->SetSelection(0);
  for (int outc = 1; outc <= p_nfg->NumOutcomes(); outc++) {
    gbtGameOutcome outcome = p_nfg->GetOutcome(outc);
    gbtText item = ToText(outc) + ": " + outcome->GetLabel();
    if (item == "") {
      item = "Outcome" + ToText(outc);
    }

    item += (" (" + ToText(outcome->GetPayoff(p_nfg->GetPlayer(1))) + ", " +
	     ToText(outcome->GetPayoff(p_nfg->GetPlayer(2))));
    if (p_nfg->NumPlayers() > 2) {
      item += ", " + ToText(outcome->GetPayoff(p_nfg->GetPlayer(3)));
      if (p_nfg->NumPlayers() > 3) {
	item += ",...)";
      }
      else {
	item += ")";
      }
    }
    else {
      item += ")";
    }

    m_outcome->Append(wxString::Format(wxT("%s"), (const char *) item));
    if (profile->GetOutcome() == outcome) {
      m_outcome->SetSelection(outc);
    }
  }
  outcomeSizer->Add(m_outcome, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(outcomeSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, _("Help")), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}
