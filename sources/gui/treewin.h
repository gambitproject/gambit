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

class TreeWindow : public wxScrolledWindow {
friend class EfgPrintout;
friend class EfgShow;
private:
  gbtGameDocument *m_doc;
  efgTreeLayout m_layout;
  TreeDrawSettings m_drawSettings;

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
    
public:
  TreeWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~TreeWindow();
    
  virtual void OnDraw(wxDC &dc);
  void OnDraw(wxDC &, double);

  void RefreshTree(void);
  void RefreshLayout(void);
  void RefreshLabels(void);
  
  void SupportChanged(void);
  
  TreeDrawSettings &DrawSettings(void) { return m_drawSettings; }
  const TreeDrawSettings &DrawSettings(void) const { return m_drawSettings; }
  
  void SetCursorPosition(gbtEfgNode p_cursor);
  void SetCutNode(gbtEfgNode, bool);
    
  void UpdateMenus(void);
  
  double GetZoom(void) const { return m_zoom; }
  void SetZoom(double p_zoom);
  void FitZoom(void);

  // Access to the numeric values from the renderer
  gText OutcomeAsString(const gbtEfgNode &n) const;

  DECLARE_EVENT_TABLE()
};

#endif   // TREEWIN_H

