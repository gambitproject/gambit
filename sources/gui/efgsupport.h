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

#include <wx/treectrl.h>
#include "efgshow.h"
#include "gamedoc.h"
#include "base/gmap.h"

//
// This is a generic tree control widget that graphically represents
// a support in an extensive form game.  Actions not in the support are
// drawn in grey, actions in the support are drawn in black.
//
class gbtEfgSupportWidget : public wxTreeCtrl {
private:
  gbtOrdMap<wxTreeItemId, gbtGameAction> m_map;

  void OnTreeItemCollapse(wxTreeEvent &);

public:
  gbtEfgSupportWidget(wxWindow *p_parent, wxWindowID p_id);

  void SetSupport(const gbtEfgSupport &);
  gbtGameAction GetAction(wxTreeItemId p_id) { return m_map(p_id); }
};

class gbtEfgSupportWindow;

class gbtEfgSupportFrame : public wxFrame, public gbtGameView {
private:
  gbtEfgSupportWindow *m_panel;

  // Event handlers
  void OnClose(wxCloseEvent &);
  
  // Overriding view members
  bool IsEfgView(void) const { return true; }
  bool IsNfgView(void) const { return false; }
  void OnUpdate(gbtGameView *);

public:
  gbtEfgSupportFrame(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtEfgSupportFrame();

  DECLARE_EVENT_TABLE()
};

#endif  // EFGSUPPORT_H
