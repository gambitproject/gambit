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

#ifndef PANEL_NASH_H
#define PANEL_NASH_H

#include "game-document.h"

class gbtNashPanel;

class gbtNashPanel : public wxPanel, public gbtGameView {
private:
  wxStaticText *m_text;
  wxButton *m_startButton, *m_cancelButton;
  wxThread *m_thread;
  gbtList<gbtMixedProfile<double> > m_eqa;

  void OnUpdate(void);

  void OnStartButton(wxCommandEvent &);
  void OnCancelButton(wxCommandEvent &);
  void OnThreadDone(wxCommandEvent &);
  
public:
  gbtNashPanel(wxWindow *p_parent, gbtGameDocument *);

  DECLARE_EVENT_TABLE()
};

#endif  // PANEL_NASH_H
