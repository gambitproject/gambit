//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of panel for controlling QRE computation
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

#ifndef PANEL_QRE_H
#define PANEL_QRE_H

#include "game-document.h"

class gbtMixedLogitBranch;
class gbtMixedLogitCtrl;
class gbtMixedLogitGraph;

class gbtQrePanel : public wxPanel, public gbtGameView {
private:
  wxButton *m_startButton;
  gbtMixedLogitCtrl *m_profileCtrl;
  gbtMixedLogitGraph *m_branchGraph;
  wxThread *m_thread;
  gbtMixedLogitBranch *m_cor;
 
  // Overriding gbtGameView members
  void OnUpdate(void) { }

  // Event handlers
  void OnStartButton(wxCommandEvent &);
  void OnThreadDone(wxCommandEvent &);

public:
  gbtQrePanel(wxWindow *p_parent, gbtGameDocument *);
  virtual ~gbtQrePanel();

  void SetCorrespondence(const gbtMixedLogitBranch &);
  
  DECLARE_EVENT_TABLE()
};


#endif   // PANEL_QRE_H
