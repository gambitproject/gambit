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

#include <cmath>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/panel.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>

#include "gambit.h"
#include "dlabout.h"
#include "bitmaps/gambitbig.xpm"

namespace Gambit::GUI {

static wxBitmap ScaleBitmapToMaxDIP(wxWindow *w, const wxBitmap &src, int maxDip)
{
  const int maxPx = w->FromDIP(maxDip);

  wxImage img = src.ConvertToImage();
  const int sw = img.GetWidth();
  const int sh = img.GetHeight();

  if (sw <= 0 || sh <= 0) {
    return src;
  }

  const int smax = std::max(sw, sh);
  if (smax <= maxPx) {
    return src;
  }

  const double scale = double(maxPx) / double(smax);
  const int nw = std::max(1, int(std::lround(sw * scale)));
  const int nh = std::max(1, int(std::lround(sh * scale)));

  img.Rescale(nw, nh, wxIMAGE_QUALITY_HIGH);
  return wxBitmap(img);
}

AboutDialog::AboutDialog(wxWindow *parent)
  : wxDialog(parent, wxID_ANY, _("About Gambit"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  const int M = FromDIP(20);
  const int S = FromDIP(10);
  const int XS = FromDIP(6);

  // Brand colour sampled earlier (adjust if desired)
  const wxColour kBrandGreen(54, 100, 96);

  auto *panel = new wxPanel(this);

  auto *root = new wxBoxSizer(wxVERTICAL);
  auto *top = new wxBoxSizer(wxHORIZONTAL);

  // --- Left brand rail (fixed width, centered logo) ---
  auto *left = new wxPanel(panel);
  left->SetBackgroundColour(kBrandGreen);

  const int railW = FromDIP(300); // tweak: 260–320 DIP
  left->SetMinSize(wxSize(railW, -1));

  wxBitmap raw(gambitbig_xpm);
  wxBitmap scaled = ScaleBitmapToMaxDIP(panel, raw, 180); // tweak: 160–200 DIP
  auto *logo = new wxStaticBitmap(left, wxID_ANY, scaled);

  auto *leftSizer = new wxBoxSizer(wxVERTICAL);
  leftSizer->AddStretchSpacer();
  leftSizer->Add(logo, 0, wxALIGN_CENTER);
  leftSizer->AddStretchSpacer();
  left->SetSizer(leftSizer);

  top->Add(left, 0, wxEXPAND | wxRIGHT, M);

  // --- Text column (right) ---
  auto *textCol = new wxBoxSizer(wxVERTICAL);

  // Tagline (keep, but wrap a bit narrower to avoid dialog getting wide)
  auto *tagline =
      new wxStaticText(panel, wxID_ANY, _("The package for computation in game theory"));
  tagline->Wrap(FromDIP(360));
  textCol->Add(tagline, 0, wxBOTTOM, S);

  // Version line
  textCol->Add(new wxStaticText(panel, wxID_ANY, _("Version " VERSION)), 0, wxBOTTOM, S);

  // Built with line + link directly underneath (keeps width down)
  textCol->Add(new wxStaticText(panel, wxID_ANY, _("Built with " wxVERSION_STRING)), 0, wxBOTTOM,
               XS);

  textCol->Add(new wxHyperlinkCtrl(panel, wxID_ANY, "wxwidgets.org", "https://www.wxwidgets.org"),
               0, wxBOTTOM, M);

  // Copyright + project link grouped together
  textCol->Add(new wxStaticText(panel, wxID_ANY, _("Copyright © 1994–2026 The Gambit Project")), 0,
               wxBOTTOM, XS);

  textCol->Add(
      new wxHyperlinkCtrl(panel, wxID_ANY, "gambit-project.org", "https://www.gambit-project.org"),
      0, wxBOTTOM, S);

  // If you still want maintainer/email, keep it—otherwise remove these two blocks.
  textCol->Add(new wxStaticText(panel, wxID_ANY, _("Theodore Turocy, Project Maintainer")), 0,
               wxBOTTOM, XS);

  textCol->Add(
      new wxHyperlinkCtrl(panel, wxID_ANY, "T.Turocy@uea.ac.uk", "mailto:T.Turocy@uea.ac.uk"), 0,
      wxBOTTOM, M);

  // License (single wrapped paragraph; narrower wrap helps overall width)
  auto *license = new wxStaticText(panel, wxID_ANY,
                                   _("This program is free software, distributed under the terms "
                                     "of the GNU General Public License."));
  license->Wrap(FromDIP(360));
  textCol->Add(license, 0, wxBOTTOM, 0);

  top->Add(textCol, 1, wxEXPAND);

  root->Add(top, 1, wxALL | wxEXPAND, M);

  root->Add(CreateStdDialogButtonSizer(wxOK), 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_RIGHT, M);

  panel->SetSizer(root);

  auto *dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->Add(panel, 1, wxEXPAND);
  SetSizerAndFit(dlgSizer);
  CentreOnParent();
}

} // namespace Gambit::GUI
