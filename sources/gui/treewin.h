//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of window class to display extensive form tree
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

#ifndef TREEWIN_H
#define TREEWIN_H

#include "wx/dragimag.h"
#include "efglayout.h"
#include "gamedoc.h"

class EfgShow;

class TreeWindow : public wxScrolledWindow, public gbtGameView {
friend class EfgPrintout;
friend class EfgShow;
private:
  efgTreeLayout m_layout;
  double m_zoom;
  wxMenu *m_nodeMenu, *m_gameMenu;

  wxDragImage *m_dragImage;
  gbtEfgNode m_dragSource;
  enum { dragCOPY, dragMOVE, dragOUTCOME } m_dragMode;

  // Private Functions
  void MakeFlasher(void);
  void UpdateCursor(void);
  void ProcessCursor(void);
  void EnsureCursorVisible(void);

  void MakeMenus(void);
  void AdjustScrollbarSteps(void);

  // Event handlers
  void OnSize(wxSizeEvent &);
  void OnMouseMotion(wxMouseEvent &);
  void OnLeftClick(wxMouseEvent &);
  void OnRightClick(wxMouseEvent &);
  void OnLeftDoubleClick(wxMouseEvent &);
  void OnKeyEvent(wxKeyEvent &);

  bool IsEfgView(void) const { return true; }
  bool IsNfgView(void) const { return false; }
  void OnUpdate(gbtGameView *);
    
public:
  TreeWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~TreeWindow();
    
  virtual void OnDraw(wxDC &dc);
  void OnDraw(wxDC &, double);

  void SetCutNode(gbtEfgNode, bool);
    
  double GetZoom(void) const { return m_zoom; }
  void SetZoom(double p_zoom);
  void FitZoom(void);

  // Access to the numeric values from the renderer
  gText OutcomeAsString(const gbtEfgNode &n) const;

  DECLARE_EVENT_TABLE()
};

//
// Tree-editing commands
// These are placed here because they're used by multiple windows currently.
// They may deserve their own file down the road.
//

//
// Move the subtree rooted at 'src' to 'dest'
//
class gbtCmdMoveTree : public gbtGameCommand {
private:
  gbtEfgNode m_src, m_dest;

public:
  gbtCmdMoveTree(gbtEfgNode p_src, gbtEfgNode p_dest)
    : m_src(p_src), m_dest(p_dest) { }
  virtual ~gbtCmdMoveTree() { }

  void Do(gbtGameDocument *);
};

//
// Copy the subtree rooted at 'src' to 'dest'
//
class gbtCmdCopyTree : public gbtGameCommand {
private:
  gbtEfgNode m_src, m_dest;

public:
  gbtCmdCopyTree(gbtEfgNode p_src, gbtEfgNode p_dest)
    : m_src(p_src), m_dest(p_dest) { }
  virtual ~gbtCmdCopyTree() { }

  void Do(gbtGameDocument *);
};

//
// Set the outcome of the 'node' to 'outcome'
//
class gbtCmdSetOutcome : public gbtGameCommand {
private:
  gbtEfgNode m_node;
  gbtEfgOutcome m_outcome;

public:
  gbtCmdSetOutcome(gbtEfgNode p_node, gbtEfgOutcome p_outcome) 
    : m_node(p_node), m_outcome(p_outcome) { }
  virtual ~gbtCmdSetOutcome() { }

  void Do(gbtGameDocument *);
};


#endif   // TREEWIN_H

