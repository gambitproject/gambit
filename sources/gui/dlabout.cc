//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of "about" dialog
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

#include "dlabout.h"
#include "bitmaps/gambit.xpm"

dialogAbout::dialogAbout(wxWindow *p_parent,
			 const wxString &p_title,
			 const wxString &p_programName,
			 const wxString &p_versionString)
  : wxDialog(p_parent, -1, p_title, wxDefaultPosition, wxDefaultSize)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  topSizer->Add(new wxStaticBitmap(this, -1, wxBITMAP(gambit)), 0, wxALL, 5);
  topSizer->Add(new wxStaticText(this, -1, p_programName),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, p_versionString),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, _("Part of the Gambit Project")),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, 
				 wxT("http://econweb.tamu.edu/gambit")),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, wxT("gambit@econmail.tamu.edu")),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, -1, 
				 wxString(_("Built with ")) +
				 wxString(wxVERSION_STRING)),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, wxT("http://www.wxwindows.org")),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, -1, _("Copyright (C) 2003")),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, _("The Gambit Project")),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1,
				 wxT("Theodore Turocy, Project Maintainer")),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, -1,
				 _("This program is free software,")),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1,
				 _("distributed under the terms of")),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, 
				 _("the GNU General Public License")),
		0, wxALIGN_CENTER, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  topSizer->Add(okButton, 0, wxALL | wxALIGN_CENTER, 10);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}
