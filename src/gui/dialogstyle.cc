//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dialogstyle.cc
// Shared layout and presentation conventions for dialogs
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
#include <wx/settings.h>

#include "dialogstyle.h"

namespace Gambit::GUI {

namespace {
/// Width, in device-independent pixels, at which an alert's text wraps
constexpr int c_alertTextWidth = 400;

bool IsDarkAppearance() { return wxSystemSettings::GetAppearance().IsDark(); }
} // namespace

void SetDialogLayout(wxDialog *p_dialog, wxSizer *p_content, long p_buttons, wxSizer *p_extra)
{
  const int margin = p_dialog->FromDIP(c_spaceMargin);
  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(p_content, 1, wxEXPAND | wxALL, margin);

  wxSizer *standard = (p_buttons != 0) ? p_dialog->CreateStdDialogButtonSizer(p_buttons) : nullptr;
  if (standard || p_extra) {
    auto *row = new wxBoxSizer(wxHORIZONTAL);
    if (p_extra) {
      row->Add(p_extra, 0, wxALIGN_CENTER_VERTICAL);
    }
    row->AddStretchSpacer();
    if (standard) {
      row->Add(standard, 0, wxALIGN_CENTER_VERTICAL);
    }
    topSizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, margin);
  }

  p_dialog->SetSizerAndFit(topSizer);
  p_dialog->CentreOnParent();
}

void RefreshDialogMinSize(wxDialog *p_dialog)
{
  p_dialog->SetMinClientSize(p_dialog->GetSizer()->ComputeFittingClientSize(p_dialog));
  wxSize size = p_dialog->GetSize();
  size.IncTo(p_dialog->GetMinSize());
  p_dialog->SetSize(size);
  p_dialog->Layout();
}

wxFlexGridSizer *NewFormSizer(const wxWindow *p_parent)
{
  const int gap = p_parent->FromDIP(c_spaceRelated);
  auto *sizer = new wxFlexGridSizer(2, gap, gap);
  sizer->AddGrowableCol(1, 1);
  return sizer;
}

wxStaticText *NewErrorText(wxWindow *p_parent)
{
  auto *text = new wxStaticText(p_parent, wxID_STATIC, wxEmptyString);
  text->SetForegroundColour(ErrorTextColour());
  return text;
}

wxColour ErrorTextColour()
{
  return IsDarkAppearance() ? wxColour(255, 105, 97) : wxColour(204, 0, 0);
}

wxColour InvalidFieldBackground()
{
  return IsDarkAppearance() ? wxColour(110, 40, 40) : wxColour(255, 220, 220);
}

wxColour NewLabelColour()
{
  return IsDarkAppearance() ? wxColour(120, 210, 120) : wxColour(0, 102, 0);
}

wxColour RenamedLabelColour()
{
  return IsDarkAppearance() ? wxColour(130, 170, 255) : wxColour(0, 51, 204);
}

void ShowOperationError(wxWindow *p_parent, const wxString &p_title, const wxString &p_headline,
                        const wxString &p_detail)
{
  AlertDialog(p_parent, p_title, wxART_ERROR, p_headline, p_detail, wxOK).ShowModal();
}

bool ConfirmOperation(wxWindow *p_parent, const wxString &p_title, const wxString &p_headline,
                      const wxString &p_detail, const wxString &p_proceedLabel)
{
  AlertDialog dialog(p_parent, p_title, wxART_WARNING, p_headline, p_detail, wxOK | wxCANCEL);
  dialog.SetButtonLabel(wxID_OK, p_proceedLabel);
  dialog.SetDefaultButton(wxID_CANCEL);
  return dialog.ShowModal() == wxID_OK;
}

void ShowInternalError(wxWindow *p_parent, const std::string &p_message)
{
  AlertDialog(p_parent, _("Internal error"), wxART_ERROR,
              _("Gambit encountered an internal error."),
              wxString::FromUTF8(p_message) + "\n\n" +
                  _("The operation was abandoned. Gambit will continue running."),
              wxOK)
      .ShowModal();
}

AlertDialog::AlertDialog(wxWindow *p_parent, const wxString &p_title, const wxArtID &p_icon,
                         const wxString &p_headline, const wxString &p_detail, long p_buttons)
  : wxDialog(p_parent, wxID_ANY, p_title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
  const int margin = FromDIP(c_spaceAlertMargin);

  auto *contentSizer = new wxBoxSizer(wxHORIZONTAL);
  contentSizer->Add(new wxStaticBitmap(this, wxID_ANY,
                                       wxArtProvider::GetBitmapBundle(p_icon, wxART_MESSAGE_BOX)),
                    0, wxALIGN_TOP | wxRIGHT, margin);

  auto *textSizer = new wxBoxSizer(wxVERTICAL);
  auto *headline = new wxStaticText(this, wxID_ANY, p_headline);
  headline->SetFont(headline->GetFont().Bold());
  headline->Wrap(FromDIP(c_alertTextWidth));
  textSizer->Add(headline, 0, wxBOTTOM, FromDIP(c_spaceRelated));
  auto *detail = new wxStaticText(this, wxID_ANY, p_detail);
  detail->Wrap(FromDIP(c_alertTextWidth));
  textSizer->Add(detail, 0);
  contentSizer->Add(textSizer, 1, wxEXPAND);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(contentSizer, 1, wxEXPAND | wxALL, margin);
  topSizer->Add(CreateStdDialogButtonSizer(p_buttons), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,
                margin);

  SetSizerAndFit(topSizer);
  CentreOnParent();
}

void AlertDialog::SetButtonLabel(wxWindowID p_id, const wxString &p_label)
{
  if (auto *button = FindWindow(p_id)) {
    button->SetLabel(p_label);
    GetSizer()->SetSizeHints(this);
    CentreOnParent();
  }
}

void AlertDialog::SetDefaultButton(wxWindowID p_id)
{
  if (auto *button = dynamic_cast<wxButton *>(FindWindow(p_id))) {
    button->SetDefault();
    button->SetFocus();
  }
}

} // namespace Gambit::GUI
