//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlabout.cc
// Implementation of "about" dialog
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

#include "dlabout.h"
#include "bitmaps/gambitbig.xpm"

static wxStaticText *FormattedText(wxWindow *p_parent, const wxString &p_label,
				   const wxFont &p_font)
{
  wxStaticText *t = new wxStaticText(p_parent, wxID_STATIC, p_label);
  t->SetFont(p_font);
  return t;
}

gbtAboutDialog::gbtAboutDialog(wxWindow *p_parent)
  : wxDialog(p_parent, -1, _T("About Gambit..."), 
	     wxDefaultPosition, wxDefaultSize)
{
  SetFont(wxFont(12, wxROMAN, wxNORMAL, wxBOLD));
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  topSizer->Add(new wxStaticBitmap(this, wxID_STATIC, 
				   wxBitmap(gambitbig_xpm)),
		0, wxALL | wxALIGN_CENTER, 20);
  topSizer->Add(new wxStaticText(this, wxID_STATIC, 
				 _T("Gambit: Software Tools for Game Theory")),
		0, wxTOP | wxLEFT | wxRIGHT | wxALIGN_CENTER, 20);
  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 _T("Graphical Interface")),
		0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, wxID_STATIC, _T("Version " wxT(VERSION))),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(FormattedText(this, 
			      _T("http://www.gambit-project.org"),
			      wxFont(12, wxMODERN, wxNORMAL, wxNORMAL)),
		0, wxTOP | wxALIGN_CENTER, 10);

  topSizer->Add(FormattedText(this, _("Built with " wxVERSION_STRING),
			      wxFont(12, wxROMAN, wxNORMAL, wxNORMAL)),
		0, wxTOP | wxALIGN_CENTER, 20);
  topSizer->Add(FormattedText(this, _T("http://www.wxwidgets.org"),
			      wxFont(12, wxMODERN, wxNORMAL, wxNORMAL)),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(FormattedText(this, 
			      _T("Copyright (C) 1994-2013, The Gambit Project"),
			      wxFont(12, wxROMAN, wxNORMAL, wxNORMAL)),
		0, wxTOP | wxALIGN_CENTER, 20);
  topSizer->Add(FormattedText(this, 
			      _("Theodore Turocy, Project Maintainer"),
			      wxFont(12, wxROMAN, wxNORMAL, wxNORMAL)),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(FormattedText(this, 
			      _T("T.Turocy@uea.ac.uk"),
			      wxFont(12, wxMODERN, wxNORMAL, wxNORMAL)),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(FormattedText(this, _("This program is free software,"),
			      wxFont(12, wxROMAN, wxNORMAL, wxNORMAL)),
		0, wxTOP | wxALIGN_CENTER, 20);
  topSizer->Add(FormattedText(this, _("distributed under the terms of"),
			      wxFont(12, wxROMAN, wxNORMAL, wxNORMAL)),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(FormattedText(this, _("the GNU General Public License"),
			      wxFont(12, wxROMAN, wxNORMAL, wxNORMAL)),
		0, wxALIGN_CENTER, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  topSizer->Add(okButton, 0, wxALL | wxALIGN_RIGHT, 20);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}
