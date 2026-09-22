//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlpreferences.cc
// Implementation of the Preferences dialog (interface language).
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

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/config.h>

#include "dlpreferences.h"
#include "app.h"

namespace Gambit::GUI {

BEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
EVT_BUTTON(wxID_OK, PreferencesDialog::OnOK)
END_EVENT_TABLE()

PreferencesDialog::PreferencesDialog(wxWindow *p_parent)
  : wxDialog(p_parent, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE)
{
  const int S = FromDIP(5);

  // Read the currently active language so the dialog opens with the current
  // choice selected.
  wxString language = wxGetApp().GetLanguage();
  if (language.empty() || language == "System") {
    language = "System";
  }

  auto *sizer = new wxBoxSizer(wxVERTICAL);

  auto *grid = new wxFlexGridSizer(2, S, S);
  grid->AddGrowableCol(1, 1);

  grid->Add(new wxStaticText(this, wxID_STATIC, _("Language:")), 0,
            wxALIGN_CENTER_VERTICAL | wxALL, S);
  m_languageChoice = new wxChoice(this, wxID_ANY);
  m_languageChoice->Append(_("System (default)"));
  m_languageChoice->Append(_("English"));
  m_languageChoice->Append(_("Spanish"));
  m_languageChoice->Append(_("French"));
  if (language == "en") {
    m_languageChoice->SetSelection(1);
  }
  else if (language == "es") {
    m_languageChoice->SetSelection(2);
  }
  else if (language == "fr") {
    m_languageChoice->SetSelection(3);
  }
  else {
    m_languageChoice->SetSelection(0);
  }
  grid->Add(m_languageChoice, 0, wxEXPAND | wxALL, S);

  sizer->Add(grid, 0, wxALL | wxEXPAND, S);

  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, S);

  SetSizerAndFit(sizer);
  CentreOnParent();
}

void PreferencesDialog::OnOK(wxCommandEvent &p_event)
{
  // Language
  wxString language = "System";
  switch (m_languageChoice->GetSelection()) {
  case 1:
    language = "en";
    break;
  case 2:
    language = "es";
    break;
  case 3:
    language = "fr";
    break;
  default:
    language = "System";
    break;
  }
  if (wxGetApp().SetLanguage(language) && wxConfigBase::Get()) {
    wxConfigBase::Get()->Write(_T("/General/Language"), language);
  }

  p_event.Skip();
}

} // namespace Gambit::GUI
