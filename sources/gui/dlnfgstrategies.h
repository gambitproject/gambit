//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to inspect and edit normal form strategies
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

#ifndef DLNFGSTRATEGIES_H
#define DLNFGSTRATEGIES_H

#include <wx/grid.h>
#include "gamedoc.h"

class dialogStrategies : public wxDialog {
private:
  gbtGame m_nfg;
  wxListBox *m_playerList;
  wxGrid *m_editGrid;
  int m_selection;
  gbtBlock<gbtArray<gbtText> > m_strategyNames;

  // Event handlers
  void OnSelChanged(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogStrategies(wxWindow *, const gbtGame &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  gbtGameCommand *GetCommand(void) const;

  DECLARE_EVENT_TABLE()
};

#endif   // DLNFGSTRATEGIES_H
