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
#include "base/gmap.h"

class gbtEfgSetSupportCommand : public gbtGameCommand {
private:
  gbtGameDocument *m_game;
  int m_index;

public:
  gbtEfgSetSupportCommand(gbtGameDocument *p_game, int m_index);
  virtual ~gbtEfgSetSupportCommand() { }

  bool Do(void);
  bool Undo(void) { return false; }
};

class gbtEfgDuplicateSupportCommand : public gbtGameCommand {
private:
  gbtGameDocument *m_game;
  int m_index;

public:
  gbtEfgDuplicateSupportCommand(gbtGameDocument *p_game, int m_index);
  virtual ~gbtEfgDuplicateSupportCommand() { }

  bool Do(void);
  bool Undo(void) { return false; }
};

class gbtEfgDeleteSupportCommand : public gbtGameCommand {
private:
  gbtGameDocument *m_game;

public:
  gbtEfgDeleteSupportCommand(gbtGameDocument *p_game);
  virtual ~gbtEfgDeleteSupportCommand() { }

  bool Do(void);
  bool Undo(void) { return false; }
};

class EfgSupportWindow : public wxPanel, public gbtGameView {
private:
  EfgShow *m_parent;
  wxChoice *m_supportList;
  wxButton *m_prevButton, *m_nextButton;
  wxTreeCtrl *m_actionTree;
  gOrdMap<wxTreeItemId, Action *> m_map;
  wxMenu *m_menu;

  void OnSupportList(wxCommandEvent &);
  void OnSupportPrev(wxCommandEvent &);
  void OnSupportNext(wxCommandEvent &);

  void OnTreeItemCollapse(wxTreeEvent &);

  void OnRightClick(wxMouseEvent &);

  virtual void OnUpdate(gbtGameView *);

public:
  EfgSupportWindow(gbtGameDocument *p_game,
		   EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgSupportWindow() { }

  int GetSupport(void) const { return m_supportList->GetSelection(); }
  void ToggleItem(wxTreeItemId);

  DECLARE_EVENT_TABLE()
};

#endif  // EFGSUPPORT_H











