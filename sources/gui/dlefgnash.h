//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for selecting algorithms to compute Nash equilibria
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

#ifndef DLEFGNASH_H
#define DLEFGNASH_H

#include "wx/treectrl.h"
#include "base/gmap.h"
#include "nash/efgalgorithm.h"

class panelEfgNashAlgorithm;

class dialogEfgNash : public wxDialog {
private:
  wxTreeCtrl *m_algorithmTree;
  wxPanel *m_currentPanel;
  wxBoxSizer *m_algPanelSizer;
  gOrdMap<wxTreeItemId, panelEfgNashAlgorithm *> m_algorithms;

  // Private auxiliary functions
  int LoadAlgorithms(const efgGame &);

  // Event handlers
  void OnSelectionChanging(wxTreeEvent &);
  void OnItemCollapsing(wxTreeEvent &);

public:
  // Lifecycle
  dialogEfgNash(wxWindow *, const EFSupport &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  efgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

#endif   // DLEFGNASH_H
