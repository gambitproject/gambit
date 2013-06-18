//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlefglogit.h
// Dialog for monitoring logit equilibrium computation
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

#ifndef DLEFGLOGIT_H
#define DLEFGLOGIT_H

#include <wx/process.h>
#include "gamedoc.h"

class gbtLogitBehavList;

class gbtLogitBehavDialog : public wxDialog {
private:
  gbtGameDocument *m_doc;
  int m_pid;
  wxProcess *m_process;
  gbtLogitBehavList *m_behavList;
  wxStaticText *m_statusText;
  wxButton *m_stopButton, *m_okButton, *m_saveButton;
  wxTimer m_timer;
  wxString m_output;
    
  void OnStop(wxCommandEvent &);
  void OnTimer(wxTimerEvent &);
  void OnIdle(wxIdleEvent &);
  void OnEndProcess(wxProcessEvent &);
  void OnSave(wxCommandEvent &);

  void Start(void);

public:
  gbtLogitBehavDialog(wxWindow *p_parent, gbtGameDocument *p_doc);

  DECLARE_EVENT_TABLE()
};

#endif  // DLLOGIT_H
