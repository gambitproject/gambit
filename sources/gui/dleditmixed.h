//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for editing properties of mixed profiles
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

#ifndef DLEDITMIXED_H
#define DLEDITMIXED_H

#include "wx/grid.h"

class dialogEditMixed : public wxDialog {
private:
  mutable MixedSolution m_profile;
  wxTextCtrl *m_profileName;
  wxListBox *m_playerList;
  wxGrid *m_probGrid;
  int m_selection;

  // Event handlers
  void OnSelChanged(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  dialogEditMixed(wxWindow *p_parent, const MixedSolution &p_profile);
  virtual ~dialogEditMixed();

  const MixedSolution &GetProfile(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // MIXEDEDIT_H
