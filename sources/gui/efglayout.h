//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to tree layout representation
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

#ifndef EFGLAYOUT_H
#define EFGLAYOUT_H

#include "base/base.h"
#include "efgconst.h"
#include "game/efg.h"
#include "gamedoc.h"

class NodeEntry {
private:
  gbtEfgNode m_node;        // the corresponding node in the game
  NodeEntry *m_parent; // parent node
  int m_x, m_y;        // Cartesian coordinates of node
  NodeEntry *m_nextMember;  // entry of next information set member 
  bool m_inSupport;    // true if node reachable in current support
  bool m_selected;    // true if node is selected
  bool m_cursor;      // true if node is 'cursor'
  bool m_cut;         // true if node is in a 'cut' subtree
  bool m_subgameRoot, m_subgameMarked;
  int m_size;         // horizontal size of the node
  int m_token;        // token to draw for node
  wxColour m_color;   // color of node

  int m_branchStyle;  // lines or fork-tine
  int m_branchLabel;  // horizontal or rotated
  int m_branchLength; // length of branch (exclusive of tine, if present)

  int m_level;        // depth of the node in tree
  int m_sublevel;     // # of the infoset line on this level
  gNumber m_actionProb;  // probability incoming action is taken

  wxString m_nodeAboveLabel, m_nodeBelowLabel, m_nodeRightLabel;
  wxString m_branchAboveLabel, m_branchBelowLabel;

  wxFont m_nodeAboveFont, m_nodeBelowFont, m_nodeRightFont;
  wxFont m_branchAboveFont, m_branchBelowFont;

public:
  NodeEntry(gbtEfgNode p_parent);

  gbtEfgNode GetNode(void) const { return m_node; }

  NodeEntry *GetParent(void) const { return m_parent; }
  void SetParent(NodeEntry *p_parent) { m_parent = p_parent; }

  int X(void) const { return m_x; }
  void SetX(int p_x) { m_x = p_x; }
  int Y(void) const { return m_y; }
  void SetY(int p_y) { m_y = p_y; }

  NodeEntry *GetNextMember(void) const { return m_nextMember; }
  void SetNextMember(NodeEntry *p_member) { m_nextMember = p_member; }

  bool InSupport(void) const { return m_inSupport; }
  void SetInSupport(bool p_inSupport) { m_inSupport = p_inSupport; }

  int GetChildNumber(void) const; 

  const wxColour &GetColor(void) const { return m_color; }
  void SetColor(const wxColour &p_color) { m_color = p_color; }

  bool IsCursor(void) const { return m_cursor; }
  void SetCursor(bool p_cursor);

  bool IsSelected(void) const { return m_selected; }
  void SetSelected(bool p_selected) { m_selected = p_selected; }

  bool IsCut(void) const { return m_cut; }
  void SetCut(bool p_cut) { m_cut = p_cut; }

  bool IsSubgameRoot(void) const { return m_subgameRoot; }
  void SetSubgameRoot(bool p_root) { m_subgameRoot = p_root; }
  
  bool IsSubgameMarked(void) const { return m_subgameMarked; }
  void SetSubgameMarked(bool p_marked) { m_subgameMarked = p_marked; }

  int GetSize(void) const { return m_size; }
  void SetSize(int p_size) { m_size = p_size; }

  int GetToken(void) const { return m_token; }
  void SetToken(int p_token) { m_token = p_token; }

  int GetBranchStyle(void) const { return m_branchStyle; }
  void SetBranchStyle(int p_style) { m_branchStyle = p_style; }

  int GetBranchLabelStyle(void) const { return m_branchLabel; }
  void SetBranchLabelStyle(int p_style) { m_branchLabel = p_style; }

  int GetBranchLength(void) const { return m_branchLength; }
  void SetBranchLength(int p_length) { m_branchLength = p_length; }

  int GetLevel(void) const { return m_level; }
  void SetLevel(int p_level) { m_level = p_level; }

  int GetSublevel(void) const { return m_sublevel; }
  void SetSublevel(int p_sublevel) { m_sublevel = p_sublevel; }

  const wxString &GetNodeAboveLabel(void) const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(const wxString &p_label)
    { m_nodeAboveLabel = p_label; }

  const wxString &GetNodeBelowLabel(void) const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(const wxString &p_label)
    { m_nodeBelowLabel = p_label; }

  const wxString &GetNodeRightLabel(void) const { return m_nodeRightLabel; }
  void SetNodeRightLabel(const wxString &p_label)
    { m_nodeRightLabel = p_label; }

  const wxString &GetBranchAboveLabel(void) const 
    { return m_branchAboveLabel; }
  void SetBranchAboveLabel(const wxString &p_label)
    { m_branchAboveLabel = p_label; }

  const wxString &GetBranchBelowLabel(void) const 
    { return m_branchBelowLabel; }
  void SetBranchBelowLabel(const wxString &p_label)
    { m_branchBelowLabel = p_label; }

  const wxFont &GetNodeAboveFont(void) const { return m_nodeAboveFont; }
  void SetNodeAboveFont(const wxFont &p_font) { m_nodeAboveFont = p_font; }

  const wxFont &GetNodeBelowFont(void) const { return m_nodeBelowFont; }
  void SetNodeBelowFont(const wxFont &p_font) { m_nodeBelowFont = p_font; }

  const wxFont &GetNodeRightFont(void) const { return m_nodeRightFont; }
  void SetNodeRightFont(const wxFont &p_font) { m_nodeRightFont = p_font; }

  const wxFont &GetBranchAboveFont(void) const { return m_branchAboveFont; }
  void SetBranchAboveFont(const wxFont &p_font) { m_branchAboveFont = p_font; }

  const wxFont &GetBranchBelowFont(void) const { return m_branchBelowFont; }
  void SetBranchBelowFont(const wxFont &p_font) { m_branchBelowFont = p_font; }

  const gNumber &GetActionProb(void) const { return m_actionProb; }
  void SetActionProb(const gNumber &p_prob) { m_actionProb = p_prob; }

  bool NodeHitTest(int p_x, int p_y) const;

  void Draw(wxDC &) const;
  void DrawIncomingBranch(wxDC &) const;
};

class TreeWindow;

class efgTreeLayout {
private:
  gbtGameDocument *m_doc;
  gList<NodeEntry *> m_nodeList;
  int m_maxX, m_maxY, m_maxLevel;
  int m_infosetSpacing;

  const int c_leftMargin, c_topMargin;

  NodeEntry *GetEntry(const gbtEfgNode &) const;

  NodeEntry *NextInfoset(NodeEntry *);
  void CheckInfosetEntry(NodeEntry *);

  void BuildNodeList(const gbtEfgNode &, const EFSupport &, int);

  int LayoutSubtree(const gbtEfgNode &, const EFSupport &,
		    int &, int &, int &);
  void FillInfosetTable(const gbtEfgNode &, const EFSupport &);
  void UpdateTableInfosets(void);
  void UpdateTableParents(void);

  wxString CreateNodeLabel(const NodeEntry *, int) const;
  wxString CreateOutcomeLabel(const NodeEntry *) const;
  wxString CreateBranchLabel(const NodeEntry *, int) const;

  void RenderSubtree(wxDC &dc) const;

public:
  efgTreeLayout(gbtGameDocument *p_doc);
  virtual ~efgTreeLayout() { }

  gbtEfgNode PriorSameLevel(const gbtEfgNode &) const;
  gbtEfgNode NextSameLevel(const gbtEfgNode &) const;

  void BuildNodeList(const EFSupport &);
  void Layout(const EFSupport &);
  void GenerateLabels(void);

  void SetCutNode(const gbtEfgNode &);

  // The following member functions are for temporary compatibility only
  NodeEntry *GetNodeEntry(const gbtEfgNode &p_node) const
    { return GetEntry(p_node); }
  NodeEntry *GetValidParent(const gbtEfgNode &);
  NodeEntry *GetValidChild(const gbtEfgNode &);

  int MaxX(void) const { return m_maxX; }
  int MaxY(void) const { return m_maxY; }

  gbtEfgNode NodeHitTest(int, int) const;
  gbtEfgNode BranchHitTest(int, int) const;
  gbtEfgNode InfosetHitTest(int, int) const;

  void Render(wxDC &) const;
};

#endif  // EFGLAYOUT_H

