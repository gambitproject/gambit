//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Class to encapsulate monitoring of an external thread
//
// This file is part of Gambit
// Copyright (c) 2006, The Gambit Project
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

#include "libgambit/libgambit.h"
#include "analysis.h"
#include "monitor.h"

//=========================================================================
//                           Monitor: Lifecycle
//=========================================================================

Monitor::Monitor(gbtAnalysisOutput &p_output, const wxString &p_data)
  : m_pid(0), m_exitCode(-1),
    m_process(0), m_output(p_output), m_timer(this, wxID_ANY)
{
  m_process = new wxProcess(this, wxID_ANY);
  m_process->Redirect();

  Connect(wxEVT_IDLE, wxIdleEventHandler(Monitor::OnIdle));
  Connect(wxEVT_TIMER, wxTimerEventHandler(Monitor::OnTimer));
  Connect(wxID_ANY, wxEVT_END_PROCESS,
	  wxProcessEventHandler(Monitor::OnEndProcess));

  m_pid = wxExecute(m_output.GetCommand(), wxEXEC_ASYNC, m_process);
  wxString str(p_data);

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

  m_timer.Start(1000, false);
}

Monitor::~Monitor()
{
  if (m_process) {
    Stop();
  }
}

//=========================================================================
//                   Monitor: Interacting with the process
//=========================================================================

void Monitor::Stop(void)
{
  if (m_process) {
    // Per the wxWidgets wiki, under Windows, programs that run
    // without a console window don't respond to the more polite
    // SIGTERM, so instead we must be rude and SIGKILL it.
#ifdef __WXMSW__
    wxProcess::Kill(m_pid, wxSIGKILL);
#else
    wxProcess::Kill(m_pid, wxSIGTERM);
#endif  // __WXMSW__
  }
}

//=========================================================================
//                         Monitor: Event handling
//=========================================================================

void Monitor::OnIdle(wxIdleEvent &p_event)
{
  if (!m_process)  return;

  if (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();

    m_output.AddOutput(msg);
    p_event.RequestMore();
  }
  else {
    m_timer.Start(1000, false);
  }
}

void Monitor::OnEndProcess(wxProcessEvent &p_event)
{
  m_timer.Stop();

  while (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();

    if (msg != wxT("")) {
      m_output.AddOutput(msg);
    }
  }

  m_exitCode = p_event.GetExitCode();
  m_process = 0;
}

