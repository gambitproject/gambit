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

#ifndef MONITOR_H
#define MONITOR_H

#include <wx/process.h>

class Monitor : public wxEvtHandler {
private:
  int m_pid, m_exitCode;
  wxProcess *m_process;
  gbtAnalysisOutput &m_output;
  wxTimer m_timer;

  /// @name Event handling
  //@{
  /// Called when a timer event occurs (check for new output)
  void OnTimer(wxTimerEvent &) { wxWakeUpIdle(); }
  /// Called when an idle event occurs (check for new output)
  void OnIdle(wxIdleEvent &);
  /// Called when the process ends
  void OnEndProcess(wxProcessEvent &);
  //@}

public:
  /// @name Lifecycle
  //@{
  /// Constructor
  Monitor(gbtAnalysisOutput &p_output, const wxString &p_data);
  /// Destructor; make sure to terminate process if still running
  ~Monitor();
  //@}

  /// @name Interacting with the process
  //@{
  /// Is the process still running?
  bool IsRunning(void) const { return (m_process != 0); }
  /// Stop the process if running (otherwise, no effect)
  void Stop(void);
  /// Get the exit code returned by the process
  int GetExitCode(void) const { return m_exitCode; }
  //@}
};


#endif // MONITOR_H
