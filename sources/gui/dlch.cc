//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for computing cognitive hierarchy correspondence
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
#include "dlch.h"

dialogNfgCH::dialogNfgCH(wxWindow *p_parent, const gbtNfgSupport &p_support)
  : wxDialog(p_parent, -1, "Compute cognitive hierarchy correspondence")
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *tauBox = new wxStaticBox(this, wxID_STATIC,
					"Compute over tau values");
  wxStaticBoxSizer *tauSizer = new wxStaticBoxSizer(tauBox, wxVERTICAL);

  wxBoxSizer *minTauSizer = new wxBoxSizer(wxHORIZONTAL);
  minTauSizer->Add(new wxStaticText(this, wxID_STATIC, "Start at tau"),
		   0, wxALL | wxCENTER, 5);
  m_minTau = new wxTextCtrl(this, -1, "0");
  minTauSizer->Add(m_minTau, 0, wxALL | wxCENTER, 5);
  tauSizer->Add(minTauSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  wxBoxSizer *maxTauSizer = new wxBoxSizer(wxHORIZONTAL);
  maxTauSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop at tau"),
		   0, wxALL | wxCENTER, 5);
  m_maxTau = new wxTextCtrl(this, -1, "10");
  maxTauSizer->Add(m_maxTau, 0, wxALL | wxCENTER, 5);
  tauSizer->Add(maxTauSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  wxBoxSizer *stepTauSizer = new wxBoxSizer(wxHORIZONTAL);
  stepTauSizer->Add(new wxStaticText(this, wxID_STATIC, "Increment by"),
		    0, wxALL | wxCENTER, 5);
  m_stepTau = new wxTextCtrl(this, -1, ".01");
  stepTauSizer->Add(m_stepTau, 0, wxALL | wxCENTER, 5);
  tauSizer->Add(stepTauSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  topSizer->Add(tauSizer, 0, wxALL | wxCENTER, 5);

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
