//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efglayout.h
// Interface to tree layout representation
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

#include "gambit.h"
#include "style.h"
#include "gamedoc.h"

class gbtNodeEntry {
private:
  Gambit::GameNode m_node;    // the corresponding node in the game
  gbtNodeEntry *m_parent;     // parent node
  int m_x, m_y;               // Cartesian coordinates of node
  gbtNodeEntry *m_nextMember; // entry of next information set member
  bool m_inSupport;           // true if node reachable in current support
  int m_size;                 // horizontal size of the node
  mutable wxRect m_outcomeRect;
  mutable Gambit::Array<wxRect> m_payoffRect;
  mutable wxRect m_branchAboveRect, m_branchBelowRect;
  int m_token;      // token to draw for node
  wxColour m_color; // color of node

  int m_branchStyle;  // lines or fork-tine
  int m_branchLabel;  // horizontal or rotated
  int m_branchLength; // length of branch (exclusive of tine, if present)

  int m_level{0};      // depth of the node in tree
  int m_sublevel;      // # of the infoset line on this level
  double m_actionProb; // probability incoming action is taken

  wxString m_nodeAboveLabel, m_nodeBelowLabel;
  wxString m_branchAboveLabel, m_branchBelowLabel;

  wxFont m_nodeAboveFont, m_nodeBelowFont;
  wxFont m_branchAboveFont, m_branchBelowFont;

  const gbtStyle *m_style{nullptr};

public:
  explicit gbtNodeEntry(Gambit::GameNode p_parent);

  Gambit::GameNode GetNode() const { return m_node; }

  gbtNodeEntry *GetParent() const { return m_parent; }
  void SetParent(gbtNodeEntry *p_parent) { m_parent = p_parent; }

  int X() const { return m_x; }
  void SetX(int p_x) { m_x = p_x; }
  int Y() const { return m_y; }
  void SetY(int p_y) { m_y = p_y; }

  gbtNodeEntry *GetNextMember() const { return m_nextMember; }
  void SetNextMember(gbtNodeEntry *p_member) { m_nextMember = p_member; }

  bool InSupport() const { return m_inSupport; }
  void SetInSupport(bool p_inSupport) { m_inSupport = p_inSupport; }

  int GetChildNumber() const;

  const wxColour &GetColor() const { return m_color; }
  void SetColor(const wxColour &p_color) { m_color = p_color; }

  int GetSize() const { return m_size; }
  void SetSize(int p_size) { m_size = p_size; }

  int GetToken() const { return m_token; }
  void SetToken(int p_token) { m_token = p_token; }

  int GetBranchStyle() const { return m_branchStyle; }
  void SetBranchStyle(int p_style) { m_branchStyle = p_style; }

  int GetBranchLabelStyle() const { return m_branchLabel; }
  void SetBranchLabelStyle(int p_style) { m_branchLabel = p_style; }

  int GetBranchLength() const { return m_branchLength; }
  void SetBranchLength(int p_length) { m_branchLength = p_length; }

  int GetLevel() const { return m_level; }
  void SetLevel(int p_level) { m_level = p_level; }

  int GetSublevel() const { return m_sublevel; }
  void SetSublevel(int p_sublevel) { m_sublevel = p_sublevel; }

  const wxString &GetNodeAboveLabel() const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(const wxString &p_label) { m_nodeAboveLabel = p_label; }

  const wxString &GetNodeBelowLabel() const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(const wxString &p_label) { m_nodeBelowLabel = p_label; }

  const wxString &GetBranchAboveLabel() const { return m_branchAboveLabel; }
  void SetBranchAboveLabel(const wxString &p_label) { m_branchAboveLabel = p_label; }

  const wxString &GetBranchBelowLabel() const { return m_branchBelowLabel; }
  void SetBranchBelowLabel(const wxString &p_label) { m_branchBelowLabel = p_label; }

  const wxFont &GetNodeAboveFont() const { return m_nodeAboveFont; }
  void SetNodeAboveFont(const wxFont &p_font) { m_nodeAboveFont = p_font; }

  const wxFont &GetNodeBelowFont() const { return m_nodeBelowFont; }
  void SetNodeBelowFont(const wxFont &p_font) { m_nodeBelowFont = p_font; }

  const wxFont &GetBranchAboveFont() const { return m_branchAboveFont; }
  void SetBranchAboveFont(const wxFont &p_font) { m_branchAboveFont = p_font; }

  const wxFont &GetBranchBelowFont() const { return m_branchBelowFont; }
  void SetBranchBelowFont(const wxFont &p_font) { m_branchBelowFont = p_font; }

  const double &GetActionProb() const { return m_actionProb; }
  void SetActionProb(const double &p_prob) { m_actionProb = p_prob; }

  void SetStyle(const gbtStyle *p_style) { m_style = p_style; }

  bool NodeHitTest(int p_x, int p_y) const;
  bool OutcomeHitTest(int p_x, int p_y) const { return (m_outcomeRect.Contains(p_x, p_y)); }
  bool BranchAboveHitTest(int p_x, int p_y) const
  {
    return (m_branchAboveRect.Contains(p_x, p_y));
  }
  bool BranchBelowHitTest(int p_x, int p_y) const
  {
    return (m_branchBelowRect.Contains(p_x, p_y));
  }

  const wxRect &GetOutcomeExtent() const { return m_outcomeRect; }
  const wxRect &GetPayoffExtent(int pl) const { return m_payoffRect[pl]; }

  void Draw(wxDC &, Gambit::GameNode selection, bool p_noHints) const;
  void DrawIncomingBranch(wxDC &) const;
  void DrawOutcome(wxDC &, bool p_noHints) const;
};

class gbtEfgDisplay;

class gbtTreeLayout : public gbtGameView {
private:
  /* gbtEfgDisplay *m_parent; */
  std::list<gbtNodeEntry *> m_nodeList;
  mutable int m_maxX{0}, m_maxY{0}, m_maxLevel{0};
  int m_infosetSpacing;

  const int c_leftMargin, c_topMargin;

  gbtNodeEntry *GetEntry(const Gambit::GameNode &) const;

  gbtNodeEntry *NextInfoset(gbtNodeEntry *);
  void CheckInfosetEntry(gbtNodeEntry *);

  void BuildNodeList(const Gambit::GameNode &, const Gambit::BehaviorSupportProfile &, int);

  int LayoutSubtree(const Gambit::GameNode &, const Gambit::BehaviorSupportProfile &, int &, int &,
                    int &);
  void FillInfosetTable(const Gambit::GameNode &, const Gambit::BehaviorSupportProfile &);
  void UpdateTableInfosets();
  void UpdateTableParents();

  wxString CreateNodeLabel(const gbtNodeEntry *, int) const;
  wxString CreateBranchLabel(const gbtNodeEntry *, int) const;

  void RenderSubtree(wxDC &dc, bool p_noHints) const;

  // Overriding gbtGameView members
  void OnUpdate() override {}

public:
  gbtTreeLayout(gbtEfgDisplay *p_parent, gbtGameDocument *p_doc);
  ~gbtTreeLayout() override = default;

  Gambit::GameNode PriorSameLevel(const Gambit::GameNode &) const;
  Gambit::GameNode NextSameLevel(const Gambit::GameNode &) const;

  void BuildNodeList(const Gambit::BehaviorSupportProfile &);
  void Layout(const Gambit::BehaviorSupportProfile &);
  void GenerateLabels();

  // The following member functions are for temporary compatibility only
  gbtNodeEntry *GetNodeEntry(const Gambit::GameNode &p_node) const { return GetEntry(p_node); }
  gbtNodeEntry *GetValidParent(const Gambit::GameNode &);
  gbtNodeEntry *GetValidChild(const Gambit::GameNode &);

  int MaxX() const { return m_maxX; }
  int MaxY() const { return m_maxY; }

  Gambit::GameNode NodeHitTest(int, int) const;
  Gambit::GameNode OutcomeHitTest(int, int) const;
  Gambit::GameNode BranchAboveHitTest(int, int) const;
  Gambit::GameNode BranchBelowHitTest(int, int) const;
  Gambit::GameNode InfosetHitTest(int, int) const;

  void Render(wxDC &, bool p_noHints) const;
};

#endif // EFGLAYOUT_H
