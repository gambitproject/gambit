//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of view of extensive form tree
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#ifndef TREE_DISPLAY_H
#define TREE_DISPLAY_H

#include "game-document.h"

class gbtTreeLayout {
private:
  gbtGameDocument *m_doc;
  int m_maxX, m_maxY;
  gbtArray<int> m_nodeLevels, m_nodeSublevels, m_ycoords;
  gbtArray<int> m_levels;

  int GetX(const gbtGameNode &) const;

  void LayoutSubtree(const gbtGameNode &, int, gbtBlock<int> &);
  void DrawOutcome(wxDC &, const gbtGameNode &) const;
  void DrawSubtree(wxDC &, const gbtGameNode &) const;

public:
  gbtTreeLayout(gbtGameDocument *p_doc)
    : m_doc(p_doc) { } 

  void LayoutTree(void);
  void DrawTree(wxDC &) const;

  int GetMaxX(void) const { return m_maxX; }
  int GetMaxY(void) const { return m_maxY; }
};

class gbtTreeDisplay : public wxScrolledWindow, public gbtGameView {
private:
  gbtTreeLayout m_layout;

  // Implementation of gbtGameView members
  void OnUpdate(void);

  // Event handlers
  void OnDraw(wxDC &);

public:
  gbtTreeDisplay(wxWindow *p_parent, gbtGameDocument *p_doc);

  const gbtTreeLayout &GetLayout(void) const { return m_layout; }
};

#endif  // TREE_DISPLAY_H


