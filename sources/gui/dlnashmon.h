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
#include "monitor.h"

class wxLed;

class gbtNashMonitorPanel : public wxPanel {
private:
  gbtGameDocument *m_doc;
  wxWindow *m_profileList;
  wxLed *m_statusLed;
  wxButton *m_stopButton;
  gbtAnalysisOutput *m_output;
  Monitor *m_monitor;
  
  void Start(gbtAnalysisOutput *);
  void OnStop(wxCommandEvent &);

public:
  gbtNashMonitorPanel(wxWindow *p_parent, gbtGameDocument *p_doc,
		      gbtAnalysisOutput *p_command);

  virtual ~gbtNashMonitorPanel() { delete m_monitor; }
  
  const gbtAnalysisOutput &GetOutput(void) const { return *m_output; }
};

#endif  // DLNASHMON_H
