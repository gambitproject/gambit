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
  GBT_NODE_TOKEN_DOT = 3,
  GBT_NODE_TOKEN_DIAMOND = 4
};

enum {
  GBT_BRANCH_STYLE_FORKTINE = 0,
  GBT_BRANCH_STYLE_HORIZONTAL = 1,
  GBT_BRANCH_STYLE_ROTATED = 2
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
private:
  int m_nodeSize, m_terminalSpacing;

  int m_branchStyle;
  int m_branchLength, m_tineLength;

  int m_nodeAboveLabel, m_nodeBelowLabel;
  int m_outcomeLabel;
  int m_branchAboveLabel, m_branchBelowLabel;

  int m_chanceToken, m_playerToken, m_terminalToken;

public:
  gbtTreeLayoutOptions(void);

  int GetNodeSize(void) const { return m_nodeSize; }
  void SetNodeSize(int p_value) { m_nodeSize = p_value; }

  int GetTerminalSpacing(void) const { return m_terminalSpacing; }
  void SetTerminalSpacing(int p_value) { m_terminalSpacing = p_value; }

  int GetBranchLength(void) const { return m_branchLength; }
  void SetBranchLength(int p_value) { m_branchLength = p_value; }

  int GetTineLength(void) const { return m_tineLength; }
  void SetTineLength(int p_value) { m_tineLength = p_value; }

  int GetNodeAboveLabel(void) const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(int p_value) { m_nodeAboveLabel = p_value; }

  int GetNodeBelowLabel(void) const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(int p_value) { m_nodeBelowLabel = p_value; }

  int GetOutcomeLabel(void) const { return m_outcomeLabel; }
  void SetOutcomeLabel(int p_value) { m_outcomeLabel = p_value; }

  int GetBranchAboveLabel(void) const { return m_branchAboveLabel; }
  void SetBranchAboveLabel(int p_value) { m_branchAboveLabel = p_value; }

  int GetBranchBelowLabel(void) const { return m_branchBelowLabel; }
  void SetBranchBelowLabel(int p_value) { m_branchBelowLabel = p_value; }

  int GetBranchStyle(void) const { return m_branchStyle; }
  void SetBranchStyle(int p_value) { m_branchStyle = p_value; }

  int GetInfosetStyle(void) const { return GBT_INFOSET_CONNECT_ALL; }
  int GetInfosetJoin(void) const { return GBT_INFOSET_JOIN_CIRCLES; }

  int GetChanceToken(void) const { return m_chanceToken; }
  void SetChanceToken(int p_value) { m_chanceToken = p_value; }

  int GetPlayerToken(void) const { return m_playerToken; }
  void SetPlayerToken(int p_value) { m_playerToken = p_value; }

  int GetTerminalToken(void) const { return m_terminalToken; }
  void SetTerminalToken(int p_value) { m_terminalToken = p_value; }

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
  gbtGameNode OutcomeHitTest(int, int) const;
  gbtGameNode BranchHitTest(int, int) const;
  gbtGameNode InfosetHitTest(int, int) const;

  void DrawTree(wxDC &) const;
};

#endif  // TREE_LAYOUT_H


