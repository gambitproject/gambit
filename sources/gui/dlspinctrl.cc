//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Generic dialog featuring just a wxSpinCtrl
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
#include "dlspinctrl.h"

dialogSpinCtrl::dialogSpinCtrl(wxWindow *p_parent, const wxString &p_caption,
			       int p_min, int p_max, int p_default)
  : wxDialog(p_parent, -1, p_caption)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *spinSizer = new wxBoxSizer(wxHORIZONTAL);
  spinSizer->Add(new wxStaticText(this, -1, p_caption),
		 0, wxCENTER | wxALL, 5);
  m_spin = new wxSpinCtrl(this, -1, wxString::Format("%d", p_default),
			  wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
			  p_min, p_max, p_default);
  spinSizer->Add(m_spin, 1, wxEXPAND | wxALL, 5);
  topSizer->Add(spinSizer, 0, wxEXPAND | wxALL, 5);

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

