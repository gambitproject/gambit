//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Normal form mixed profile window
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

#ifndef NFGPROFILE_H
#define NFGPROFILE_H

#include "wx/listctrl.h"
#include "nfgshow.h"

class NfgProfileList : public wxListCtrl, public gbtGameView {
private:
  NfgShow *m_parent;
  wxMenu *m_menu;

  // Event handlers
  void OnRightClick(wxMouseEvent &);
  void OnColumnClick(wxListEvent &);

public:
  NfgProfileList(gbtGameDocument *p_game,
		 NfgShow *p_nfgShow, wxWindow *p_parent);
  virtual ~NfgProfileList();

  void UpdateValues(void);

  wxString GetReport(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // NFGPROFILE_H
