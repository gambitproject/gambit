//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for monitoring progress of logit equilibrium computation
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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
#include <wx/txtstrm.h>
#include "wxled.h"

#include "dlnashmon.h"
#include "gamedoc.h"

#include "efgprofile.h"
#include "nfgprofile.h"

#include "bitmaps/stop.xpm"

gbtNashMonitorPanel::gbtNashMonitorPanel(wxWindow *p_parent,
					 gbtGameDocument *p_doc,
					 gbtAnalysisOutput *p_command)
  : wxPanel(p_parent, wxID_ANY), m_doc(p_doc), 
    m_output(p_command)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *startSizer = new wxBoxSizer(wxHORIZONTAL);

  m_statusLed = new wxLed(this, wxID_ANY);
  m_statusLed->SetColor("0000FF");
  m_statusLed->Enable(true);
  startSizer->Add(m_statusLed, 0, wxALL | wxALIGN_CENTER, 5);

  sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, 5);

  m_stopButton = new wxBitmapButton(this, wxID_CANCEL, wxBitmap(stop_xpm));
  m_stopButton->Enable(false);
  m_stopButton->SetToolTip(_("Stop the computation"));
  startSizer->Add(m_stopButton, 0, wxALL | wxALIGN_CENTER, 5);

  Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtNashMonitorPanel::OnStop));

  if (p_command->IsBehavior()) {
    m_profileList = new gbtBehavProfileList(this, m_doc);
  }
  else {
    m_profileList = new gbtMixedProfileList(this, m_doc);
  }
  m_profileList->SetSizeHints(wxSize(500, 300));
  sizer->Add(m_profileList, 1, wxALL | wxEXPAND, 5);
  
  SetSizer(sizer);
  Layout();

  Start(p_command);
}

void gbtNashMonitorPanel::Start(gbtAnalysisOutput *p_command)
{
  if (!p_command->IsBehavior()) {
    // Make sure we have a normal form representation
    m_doc->BuildNfg();
  }

  m_doc->AddProfileList(p_command);

  std::ostringstream s;
  if (p_command->IsBehavior()) {
    m_doc->GetGame()->WriteEfgFile(s);
  }
  else {
    m_doc->GetGame()->WriteNfgFile(s);
  }
  wxString str(wxString(s.str().c_str(), *wxConvCurrent));
  
  m_monitor = new Monitor(*m_output, str);
  m_stopButton->Enable(true);
  m_statusLed->SetToolTip(wxT("The computation is in progress"));
}

void gbtNashMonitorPanel::OnStop(wxCommandEvent &p_event)
{
  m_monitor->Stop();
}

