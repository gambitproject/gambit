//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Declaration of extensive form outcome palette window
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef EFGOUTCOME_H
#define EFGOUTCOME_H

#include "wx/grid.h"
#include "efgshow.h"

class EfgOutcomeWindow : public wxGrid, public gbtGameView {
private:
  wxMenu *m_menu;

  void OnChar(wxKeyEvent &);
  void OnCellChanged(wxGridEvent &);
  void OnCellRightClick(wxGridEvent &);
  void OnLabelRightClick(wxGridEvent &);

  void OnPopupOutcomeNew(wxCommandEvent &);
  void OnPopupOutcomeDelete(wxCommandEvent &);
  void OnPopupOutcomeAttach(wxCommandEvent &);
  void OnPopupOutcomeDetach(wxCommandEvent &);

  bool IsEfgView(void) const { return true; }
  bool IsNfgView(void) const { return false; }
  void OnUpdate(gbtGameView *);
    
public:
  EfgOutcomeWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~EfgOutcomeWindow() { }

  DECLARE_EVENT_TABLE()
};

#endif   // EFGOUTCOME_H
