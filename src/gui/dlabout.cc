//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#endif // WX_PRECOMP
#include <wx/hyperlink.h>

#include "gambit.h"
#include "dlabout.h"
#include "bitmaps/gambitbig.xpm"

namespace Gambit::GUI {

AboutDialog::AboutDialog(wxWindow *p_parent)
  : wxDialog(p_parent, wxID_ANY, _T("About Gambit..."), wxDefaultPosition, wxDefaultSize)
{
  const int M = FromDIP(20);
  const int S = FromDIP(10);
  const int XS = FromDIP(5);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  topSizer->Add(new wxStaticBitmap(this, wxID_ANY, wxBitmap(gambitbig_xpm), wxDefaultPosition,
                                   wxSize(-1, 300)),
                0, wxALL | wxALIGN_CENTER, M);
  auto *title =
      new wxStaticText(this, wxID_ANY, _T("Gambit: The package for computation in game theory"));
  {
    wxFont f = title->GetFont();
    f.SetWeight(wxFONTWEIGHT_BOLD);
    f.SetPointSize(f.GetPointSize() + 2);
    title->SetFont(f);
  }
  topSizer->Add(title, 0, wxTOP | wxLEFT | wxRIGHT | wxALIGN_CENTER, M);
  topSizer->Add(new wxStaticText(this, wxID_ANY, _T("Version " VERSION)), 0, wxALIGN_CENTER, 5);

  topSizer->Add(
      new wxHyperlinkCtrl(this, wxID_ANY, "gambit-project.org", "https://www.gambit-project.org"),
      0, wxTOP | wxALIGN_CENTER, S);

  topSizer->Add(new wxStaticText(this, wxID_ANY, _("Built with " wxVERSION_STRING)), 0,
                wxTOP | wxALIGN_CENTER, M);
  topSizer->Add(new wxHyperlinkCtrl(this, wxID_ANY, "wxwidgets.org", "https://www.wxwidgets.org"),
                0, wxALIGN_CENTER, XS);

  topSizer->Add(
      new wxStaticText(this, wxID_ANY, _T("Copyright (C) 1994-2026, The Gambit Project")), 0,
      wxTOP | wxALIGN_CENTER, M);
  topSizer->Add(new wxStaticText(this, wxID_ANY, _("Theodore Turocy, Project Maintainer")), 0,
                wxALIGN_CENTER, XS);
  topSizer->Add(
      new wxHyperlinkCtrl(this, wxID_ANY, "T.Turocy@uea.ac.uk", "mailto:T.Turocy@uea.ac.uk"), 0,
      wxALIGN_CENTER, XS);

  topSizer->Add(new wxStaticText(this, wxID_ANY, _("This program is free software,")), 0,
                wxTOP | wxALIGN_CENTER, M);
  topSizer->Add(new wxStaticText(this, wxID_ANY, _("distributed under the terms of")), 0,
                wxALIGN_CENTER, XS);
  topSizer->Add(new wxStaticText(this, wxID_ANY, _("the GNU General Public License")), 0,
                wxALIGN_CENTER, XS);

  auto *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  topSizer->Add(okButton, 0, wxALL | wxALIGN_RIGHT, M);

  SetSizerAndFit(topSizer);
  CentreOnParent();
}
} // namespace Gambit::GUI
