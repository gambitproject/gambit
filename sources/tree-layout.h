//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class to layout an extensive form tree
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#ifndef TREE_LAYOUT_H
#define TREE_LAYOUT_H

#include <libgambit/libgambit.h>

class gbtGameDocument;
class gbtNodeEntry;

enum {
  GBT_NODE_TOKEN_LINE = 0,
  GBT_NODE_TOKEN_BOX = 1,
  GBT_NODE_TOKEN_CIRCLE = 2,
  GBT_NODE_TOKEN_DIAMOND = 3
};

enum {
  GBT_BRANCH_STYLE_LINE = 0,
  GBT_BRANCH_STYLE_FORKTINE = 1
};

enum {
  GBT_BRANCH_LABEL_HORIZONTAL = 0,
  GBT_BRANCH_LABEL_ROTATED = 1
};

enum {
  GBT_INFOSET_JOIN_LINES = 0,
  GBT_INFOSET_JOIN_CIRCLES = 1
};

enum {
  GBT_INFOSET_CONNECT_NONE = 0,
  GBT_INFOSET_CONNECT_SAMELEVEL = 1,
  GBT_INFOSET_CONNECT_ALL = 2
};

enum {
  GBT_LABEL_NODE_NONE = 0,
  GBT_LABEL_NODE_LABEL = 1,
  GBT_LABEL_NODE_PLAYER = 2,
  GBT_LABEL_NODE_INFOSET = 3,
  GBT_LABEL_NODE_INFOSETID = 4
};

enum {
  GBT_LABEL_OUTCOME_PAYOFFS = 0,
  GBT_LABEL_OUTCOME_LABEL = 1
};

enum {
  GBT_LABEL_BRANCH_NONE = 0,
  GBT_LABEL_BRANCH_LABEL = 1,
  GBT_LABEL_BRANCH_PROB = 2
};

class gbtTreeLayoutOptions {
public:
  int GetNodeSize(void) const { return 20; }
  int GetTerminalSpacing(void) const { return 45; }
  int GetBranchLength(void) const { return 60; }
  int GetTineLength(void) const { return 20; }

  int GetNodeAboveLabel(void) const { return GBT_LABEL_NODE_LABEL; }
  int GetNodeBelowLabel(void) const { return GBT_LABEL_NODE_INFOSETID; }
  int GetBranchAboveLabel(void) const { return GBT_LABEL_BRANCH_LABEL; }
  int GetBranchBelowLabel(void) const { return GBT_LABEL_BRANCH_PROB; }
  int GetOutcomeLabel(void) const { return GBT_LABEL_OUTCOME_PAYOFFS; }

  int GetBranchStyle(void) const { return GBT_BRANCH_STYLE_LINE; }
  int GetBranchLabelStyle(void) const { return GBT_BRANCH_LABEL_ROTATED; }

  int GetInfosetStyle(void) const { return GBT_INFOSET_CONNECT_ALL; }
  int GetInfosetJoin(void) const { return GBT_INFOSET_JOIN_CIRCLES; }

  int GetChanceToken(void) const { return GBT_NODE_TOKEN_CIRCLE; }
  int GetPlayerToken(int) const { return GBT_NODE_TOKEN_CIRCLE; }
  int GetTerminalToken(void) const { return GBT_NODE_TOKEN_CIRCLE; }

  wxFont GetTreeLabelFont(void) const
    { return wxFont(10, wxSWISS, wxNORMAL, wxBOLD); }
};


class gbtTreeLayout {
private:
  gbtGameDocument *m_doc;
  gbtBlock<gbtNodeEntry *> m_nodeList;
  int m_maxX, m_maxY, m_maxLevel;
  int m_infosetSpacing;

  const int c_leftMargin, c_topMargin;

  gbtNodeEntry *GetEntry(gbtGameNode) const;
  gbtNodeEntry *NextInfoset(gbtNodeEntry *);
  void CheckInfosetEntry(gbtNodeEntry *);

  void BuildNodeList(gbtGameNode, int);

  int LayoutSubtree(gbtGameNode, int &, int &, int &);
  void FillInfosetTable(gbtGameNode);
  void UpdateTableInfosets(void);
  void UpdateTableParents(void);

  wxString CreateNodeLabel(const gbtNodeEntry *, int) const;
  wxString CreateOutcomeLabel(const gbtNodeEntry *, int) const;
  wxString CreateBranchLabel(const gbtNodeEntry *, int) const;

  void RenderSubtree(wxDC &dc) const;

public:
  gbtTreeLayout(gbtGameDocument *);
  virtual ~gbtTreeLayout() { }

  gbtGameNode PriorSameLevel(gbtGameNode) const;
  gbtGameNode NextSameLevel(gbtGameNode) const;

  void BuildNodeList(void);
  void Layout(void);
  void GenerateLabels(void);

  void SetCutNode(gbtGameNode, bool);

  int GetMaxX(void) const { return m_maxX; }
  int GetMaxY(void) const { return m_maxY; }

  gbtGameNode NodeHitTest(int, int) const;
  gbtGameNode BranchHitTest(int, int) const;
  gbtGameNode InfosetHitTest(int, int) const;

  void DrawTree(wxDC &) const;
};

#endif  // TREE_LAYOUT_H


