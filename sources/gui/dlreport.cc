//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to show a text report
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

#include "base/gstream.h"
#include "dlreport.h"

const int idBUTTON_FILE = 2000;

BEGIN_EVENT_TABLE(dialogReport, wxDialog)
  EVT_BUTTON(idBUTTON_FILE, dialogReport::OnFile)
END_EVENT_TABLE()

dialogReport::dialogReport(wxWindow *p_parent, const wxString &p_text)
  : wxDialog(p_parent, -1, "Report")
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_text = new wxTextCtrl(this, -1, p_text, 
			  wxDefaultPosition, wxSize(600, 300),
			  wxTE_MULTILINE | wxTE_READONLY);
  wxFont font = m_text->GetFont();
  font.SetFamily(wxMODERN);
  m_text->SetFont(font);
  topSizer->Add(m_text, 1, wxALL | wxEXPAND, 5);

  topSizer->Add(new wxButton(this, idBUTTON_FILE, "Save to file..."),
		0, wxALL | wxCENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}

void dialogReport::OnFile(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Save profile report", "", "", "*.*", wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      gFileOutput file(dialog.GetPath().c_str());
      file << m_text->GetValue().c_str();
    }
    catch (gFileOutput::OpenFailed &) {
      wxMessageDialog msgDialog(this,
				wxString::Format("Could not open '%s' "
						 "for writing.",
						 dialog.GetPath().c_str()),
				"Error", wxOK | wxCENTER);
      msgDialog.ShowModal();
    }
    catch (gFileOutput::WriteFailed &) {
      wxMessageDialog msgDialog(this,
				wxString::Format("Error in writing to  '%s'.",
						 dialog.GetPath().c_str()),
				"Error", wxOK | wxCENTER);
      msgDialog.ShowModal();
    }
  }
}

