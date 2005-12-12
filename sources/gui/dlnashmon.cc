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
#include <wx/tokenzr.h>

#include "dlnashmon.h"
#include "gamedoc.h"

#include "efgprofile.h"
#include "nfgprofile.h"

class gbtNotNashException : public gbtException {
public:
  virtual ~gbtNotNashException() { }

  std::string GetDescription(void) const
  { return "Output line does not contain a Nash equilibrium"; }
};

static gbtMixedProfile<gbtNumber> TextToMixedProfile(gbtGameDocument *p_doc,
						     const wxString &p_text)
{
  gbtMixedProfile<gbtNumber> profile(p_doc->GetNfg());

  wxStringTokenizer tok(p_text, wxT(","));

  if (tok.GetNextToken() == wxT("NE")) {
    if (tok.CountTokens() == profile.Length()) {
      for (int i = 1; i <= profile.Length(); i++) {
	profile[i] = ToNumber(std::string((const char *) tok.GetNextToken().mb_str()));
      }
      return profile;
    }
  }

  throw gbtNotNashException();
}

static gbtBehavProfile<gbtNumber> TextToBehavProfile(gbtGameDocument *p_doc,
						     const wxString &p_text)
{
  gbtBehavProfile<gbtNumber> profile(*p_doc->GetEfg());

  wxStringTokenizer tok(p_text, wxT(","));

  if (tok.GetNextToken() == wxT("NE")) {
    if (tok.CountTokens() == profile.Length()) {
      for (int i = 1; i <= profile.Length(); i++) {
	profile[i] = ToNumber(std::string((const char *) tok.GetNextToken().mb_str()));
      }
      return profile;
    }
  }

  throw gbtNotNashException();
}


const int GBT_ID_TIMER = 1000;
const int GBT_ID_PROCESS = 1001;

BEGIN_EVENT_TABLE(gbtNashMonitorDialog, wxDialog)
  EVT_END_PROCESS(GBT_ID_PROCESS, gbtNashMonitorDialog::OnEndProcess)
  EVT_IDLE(gbtNashMonitorDialog::OnIdle)
  EVT_TIMER(GBT_ID_TIMER, gbtNashMonitorDialog::OnTimer)
END_EVENT_TABLE()

#include "bitmaps/stop.xpm"

gbtNashMonitorDialog::gbtNashMonitorDialog(wxWindow *p_parent,
					   gbtGameDocument *p_doc,
					   const gbtAnalysisProfileList &p_command)
  : wxDialog(p_parent, -1, wxT("Computing Nash equilibria"),
	     wxDefaultPosition),
    m_doc(p_doc), m_useBehav(p_command.GetCommand().Find(wxT("-efg-")) != -1),
    m_foundCount(0), m_process(0), m_timer(this, GBT_ID_TIMER)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *startSizer = new wxBoxSizer(wxHORIZONTAL);

  m_statusText = new wxStaticText(this, wxID_STATIC,
				  wxT("The computation is currently in progress."));
  m_statusText->SetForegroundColour(*wxBLUE);
  startSizer->Add(m_statusText, 0, wxALL | wxALIGN_CENTER, 5);

  m_countText = new wxStaticText(this, wxID_STATIC, 
				 wxT("Number of equilibria found so far: 0  "));
  startSizer->Add(m_countText, 0, wxALL | wxALIGN_CENTER, 5);
  
  m_stopButton = new wxBitmapButton(this, wxID_CANCEL, wxBitmap(stop_xpm));
  m_stopButton->Enable(false);
  m_stopButton->SetToolTip(_("Stop the computation"));
  startSizer->Add(m_stopButton, 0, wxALL | wxALIGN_CENTER, 5);

  Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtNashMonitorDialog::OnStop));

  sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, 5);

  if (m_useBehav) {
    m_profileList = new gbtBehavProfileList(this, m_doc);
  }
  else {
    m_profileList = new gbtMixedProfileList(this, m_doc);
  }
  m_profileList->SetSizeHints(wxSize(500, 300));
  sizer->Add(m_profileList, 1, wxALL | wxEXPAND, 5);
  
  m_okButton = new wxButton(this, wxID_OK, wxT("OK"));
  sizer->Add(m_okButton, 0, wxALL | wxALIGN_RIGHT, 5);
  m_okButton->Enable(false);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  Layout();
  CenterOnParent();

  Start(p_command);
}

void gbtNashMonitorDialog::Start(const gbtAnalysisProfileList &p_command)
{
  if (!m_useBehav) {
    // Make sure we have a normal form representation
    m_doc->BuildNfg();
  }

  m_doc->AddProfileList(p_command);

  m_process = new wxProcess(this, GBT_ID_PROCESS);
  m_process->Redirect();

  m_pid = wxExecute(p_command.GetCommand(), wxEXEC_ASYNC, m_process);
  
  std::ostringstream s;
  if (m_useBehav) {
    m_doc->GetEfg()->WriteEfgFile(s);
  }
  else {
    m_doc->GetNfg()->WriteNfgFile(s);
  }
  wxString str(wxString(s.str().c_str(), *wxConvCurrent));
  
  // It is possible that the whole string won't write on one go, so
  // we should take this possibility into account.  If the write doesn't
  // complete the whole way, we take a 100-millisecond siesta and try
  // again.  (This seems to primarily be an issue with -- you guessed it --
  // Windows!)
  while (str.length() > 0) {
    wxTextOutputStream os(*m_process->GetOutputStream());

    // It appears that (at least with mingw) the string itself contains
    // only '\n' for newlines.  If we don't SetMode here, these get
    // converted to '\r\n' sequences, and so the number of characters
    // LastWrite() returns does not match the number of characters in
    // our string.  Setting this explicitly solves this problem.
    os.SetMode(wxEOL_UNIX);
    os.WriteString(str);
    str.Remove(0, m_process->GetOutputStream()->LastWrite());
    wxMilliSleep(100);
  }
  m_process->CloseOutput();

  m_stopButton->Enable(true);

  m_timer.Start(1000, false);
}

void gbtNashMonitorDialog::OnIdle(wxIdleEvent &p_event)
{
  if (!m_process)  return;

  if (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();

    try {
      if (m_useBehav) {
	m_doc->AddProfile(TextToBehavProfile(m_doc, msg));
      }
      else {
	m_doc->AddProfile(TextToMixedProfile(m_doc, msg));
      }

      m_countText->SetLabel(wxString::Format(wxT("Number of equilibria found so far: %d"), ++m_foundCount));

    }
    catch (gbtNotNashException &) { }

    p_event.RequestMore();
  }
  else {
    m_timer.Start(1000, false);
  }
}

void gbtNashMonitorDialog::OnTimer(wxTimerEvent &p_event)
{
  wxWakeUpIdle();
}

void gbtNashMonitorDialog::OnEndProcess(wxProcessEvent &p_event)
{
  m_stopButton->Enable(false);
  m_timer.Stop();

  while (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();

    if (msg != wxT("")) {
      try {
	if (m_useBehav) {
	  m_doc->AddProfile(TextToBehavProfile(m_doc, msg));
	}
	else {
	  m_doc->AddProfile(TextToMixedProfile(m_doc, msg));
	}
	
	m_countText->SetLabel(wxString::Format(wxT("Number of equilibria found so far: %d"), ++m_foundCount));
      }
      catch (gbtNotNashException &) { }
    }
  }

  if (p_event.GetExitCode() == 0) {
    m_statusText->SetLabel(wxT("The computation has completed."));
    m_statusText->SetForegroundColour(wxColour(0, 192, 0));
  }
  else {
    m_statusText->SetLabel(wxT("The computation ended abnormally."));
    m_statusText->SetForegroundColour(*wxRED);
  }

  m_okButton->Enable(true);
}

void gbtNashMonitorDialog::OnStop(wxCommandEvent &p_event)
{
  // Per the wxWidgets wiki, under Windows, programs that run
  // without a console window don't respond to the more polite
  // SIGTERM, so instead we must be rude and SIGKILL it.
  m_stopButton->Enable(false);

#ifdef __WXMSW__
  wxProcess::Kill(m_pid, wxSIGKILL);
#else
  wxProcess::Kill(m_pid, wxSIGTERM);
#endif  // __WXMSW__
}

