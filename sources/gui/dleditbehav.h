//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for editing properties of behavior profiles
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

#ifndef DLEDITBEHAV_H
#define DLEDITBEHAV_H

#include "wx/treectrl.h"
#include "wx/grid.h"

#include "base/base.h"

class dialogEditBehav : public wxDialog {
private:
  mutable BehavSolution m_profile;
  gbtEfgInfoset m_lastInfoset;
  wxTextCtrl *m_profileName;
  wxTreeCtrl *m_infosetTree;
  wxGrid *m_probGrid;
  gOrdMap<wxTreeItemId, gbtEfgInfoset> m_map;

  // Event handlers
  void OnItemCollapsing(wxTreeEvent &);
  void OnSelChanging(wxTreeEvent &);
  void OnSelChanged(wxTreeEvent &);
  void OnOK(wxCommandEvent &);

public:
  dialogEditBehav(wxWindow *p_parent, const BehavSolution &p_profile);
  virtual ~dialogEditBehav();

  const BehavSolution &GetProfile(void) const;

  DECLARE_EVENT_TABLE()
};


#endif  // BEHAVEDIT_H
