//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a normal form game
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

#ifndef DLNFGPROPERTIES_H
#define DLNFGPROPERTIES_H

#include "wx/notebook.h"

class dialogNfgProperties : public wxDialog {
private:
  gbtNfgGame &m_nfg;
  wxNotebook *m_notebook;

  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogNfgProperties(wxWindow *p_parent, gbtNfgGame &p_nfg, const wxString &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetGameTitle(void) const;
  wxString GetComment(void) const;

  int NumPlayers(void) const;
  wxString GetPlayerName(int pl) const;

  DECLARE_EVENT_TABLE()
};


#endif  // DLEFGPROPERTIES_H

