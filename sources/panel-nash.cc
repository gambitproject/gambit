//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of panel for controlling equilibrium computation
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#include "panel-nash.h"

const int GBT_BUTTON_START = 997;
const int GBT_BUTTON_CANCEL = 998;
const int GBT_THREAD_DONE = 999;

class gbtAllNashThread : public wxThread {
private:
  gbtNashPanel *m_parent;
  gbtList<gbtMixedProfile<double> > &m_eqa;

public:
  gbtAllNashThread(gbtNashPanel *p_parent,
		   gbtList<gbtMixedProfile<double> > &p_eqa);

  // Overriding members to implement thread
  void *Entry(void);
  void OnExit(void);
};

gbtAllNashThread::gbtAllNashThread(gbtNashPanel *p_parent,
				   gbtList<gbtMixedProfile<double> > &p_eqa)
  : m_parent(p_parent), m_eqa(p_eqa)
{ }

void gbtAllNashThread::OnExit(void)
{ }

#include <gambit/nash/nfgpns.h>

void *gbtAllNashThread::Entry(void)
{
  m_eqa = gbtPNSNfg(m_parent->GetDocument()->GetGame(), 0);

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_THREAD_DONE);
  wxPostEvent(m_parent, event);

  return NULL;
}

BEGIN_EVENT_TABLE(gbtNashPanel, wxPanel)
  EVT_BUTTON(GBT_BUTTON_START, gbtNashPanel::OnStartButton)
  EVT_BUTTON(GBT_BUTTON_CANCEL, gbtNashPanel::OnCancelButton)
  EVT_MENU(GBT_THREAD_DONE, gbtNashPanel::OnThreadDone)
END_EVENT_TABLE()

gbtNashPanel::gbtNashPanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc),
    m_thread(0)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  m_text = new wxStaticText(this, wxID_STATIC, "Press start to run.");
  sizer->Add(m_text, 0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  
  m_startButton = new wxButton(this, GBT_BUTTON_START, "Start");
  buttonSizer->Add(m_startButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_cancelButton = new wxButton(this, GBT_BUTTON_CANCEL, "Cancel");
  buttonSizer->Add(m_cancelButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_cancelButton->Enable(false);

  sizer->Add(buttonSizer, 0, wxALIGN_CENTER, 0);
  
  SetSizer(sizer);
  Layout();
}

void gbtNashPanel::OnStartButton(wxCommandEvent &)
{
  m_startButton->Enable(false);
  m_cancelButton->Enable(true);
  m_eqa.Flush();
  m_thread = new gbtAllNashThread(this, m_eqa);
  m_thread->Create();
  if (m_thread->Run() != wxTHREAD_NO_ERROR) {
    printf("Whoops!\n");
  }
  else {
    m_text->SetLabel("Running...");
  }

}

void gbtNashPanel::OnCancelButton(wxCommandEvent &)
{
  // Doesn't actually do anything yet!
}

void gbtNashPanel::OnThreadDone(wxCommandEvent &)
{
  m_text->SetLabel("Done!");
  printf("Number of equilibria found = %d\n", m_eqa.Length());
  m_thread = 0;
  m_cancelButton->Enable(false);
  m_startButton->Enable(true);
}

void gbtNashPanel::OnUpdate(void)
{ }
