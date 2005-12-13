//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for monitoring Nash equilibrium computation progress
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

#ifndef DLNASHMON_H
#define DLNASHMON_H

#include <wx/process.h>
#include "wx/sheet/sheet.h"
#include "gamedoc.h"

class gbtNashMonitorDialog : public wxDialog {
private:
  gbtGameDocument *m_doc;
  bool m_useBehav;
  int m_pid, m_foundCount;
  wxProcess *m_process;
  wxWindow *m_profileList;
  wxStaticText *m_statusText, *m_countText;
  wxButton *m_stopButton, *m_okButton;
  wxTimer m_timer;
  
  void Start(const gbtAnalysisProfileList &);

  void OnStop(wxCommandEvent &);
  void OnTimer(wxTimerEvent &);
  void OnIdle(wxIdleEvent &);
  void OnEndProcess(wxProcessEvent &);

public:
  gbtNashMonitorDialog(wxWindow *p_parent, gbtGameDocument *p_doc,
		       const gbtAnalysisProfileList &p_command);

  DECLARE_EVENT_TABLE()
};

#endif  // DLNASHMON_H
