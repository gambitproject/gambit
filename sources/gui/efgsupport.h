//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Declaration of extensive form support palette window
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

#ifndef EFGSUPPORT_H
#define EFGSUPPORT_H

#include "wx/treectrl.h"
#include "efgshow.h"
#include "gamedoc.h"
#include "base/gmap.h"

class EfgSupportWindow : public wxPanel {
private:
  gbtGameDocument *m_doc;
  wxChoice *m_supportList;
  wxButton *m_prevButton, *m_nextButton;
  wxTreeCtrl *m_actionTree;
  gOrdMap<wxTreeItemId, gbtEfgAction> m_map;
  wxMenu *m_menu;

  void OnSupportList(wxCommandEvent &);
  void OnSupportPrev(wxCommandEvent &);
  void OnSupportNext(wxCommandEvent &);

  void OnTreeItemCollapse(wxTreeEvent &);

  void OnRightClick(wxMouseEvent &);

public:
  EfgSupportWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~EfgSupportWindow() { }

  void UpdateValues(void);

  int GetSupport(void) const { return m_supportList->GetSelection(); }
  void ToggleItem(wxTreeItemId);

  DECLARE_EVENT_TABLE()
};

#endif  // EFGSUPPORT_H
