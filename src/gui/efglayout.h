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

#ifndef GAMBIT_GUI_EFGLAYOUT_H
#define GAMBIT_GUI_EFGLAYOUT_H

#include "gambit.h"
#include "gamedoc.h"

#include "layout.h"

namespace Gambit::GUI {
class NodeEntry {
  friend class TreeLayout;
  GameNode m_node;                         // the corresponding node in the game
  std::shared_ptr<NodeEntry> m_parent;     // parent node
  int m_x{-1}, m_y{-1};                    // Cartesian coordinates of node
  std::shared_ptr<NodeEntry> m_nextMember; // entry of next information set member
  bool m_inSupport{true};                  // true if node reachable in current support
  int m_size{20};                          // horizontal size of the node
  mutable wxRect m_outcomeRect;
  mutable Array<wxRect> m_payoffRect;
  mutable wxRect m_branchAboveRect, m_branchBelowRect;

  int m_branchLength{0}; // length of branch (exclusive of tine, if present)

  int m_level{0};         // depth of the node in tree
  int m_sublevel{0};      // # of the infoset line on this level
  double m_actionProb{0}; // probability incoming action is taken

  wxString m_nodeAboveLabel, m_nodeBelowLabel;
  wxString m_branchAboveLabel, m_branchBelowLabel;

public:
  explicit NodeEntry(const GameNode &p_node) : m_node(p_node) {}
  GameNode GetNode() const { return m_node; }

  std::shared_ptr<NodeEntry> GetParent() const { return m_parent; }
  void SetParent(const std::shared_ptr<NodeEntry> &p_parent) { m_parent = p_parent; }

  int GetX() const { return m_x; }
  int GetY() const { return m_y; }

  std::shared_ptr<NodeEntry> GetNextMember() const { return m_nextMember; }

  int GetChildNumber() const
  {
    return (m_node->GetParent()) ? m_node->GetPriorAction()->GetNumber() : 0;
  }

  int GetSize() const { return m_size; }

  int GetBranchLength() const { return m_branchLength; }

  int GetLevel() const { return m_level; }
  int GetSublevel() const { return m_sublevel; }

  const wxString &GetNodeAboveLabel() const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(const wxString &p_label) { m_nodeAboveLabel = p_label; }

  const wxString &GetNodeBelowLabel() const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(const wxString &p_label) { m_nodeBelowLabel = p_label; }

  const wxString &GetBranchAboveLabel() const { return m_branchAboveLabel; }
  void SetBranchAboveLabel(const wxString &p_label) { m_branchAboveLabel = p_label; }

  const wxString &GetBranchBelowLabel() const { return m_branchBelowLabel; }
  void SetBranchBelowLabel(const wxString &p_label) { m_branchBelowLabel = p_label; }

  const double &GetActionProb() const { return m_actionProb; }
  void SetActionProb(const double &p_prob) { m_actionProb = p_prob; }

  bool OutcomeHitTest(const int p_x, const int p_y) const
  {
    return (m_outcomeRect.Contains(p_x, p_y));
  }
  bool BranchAboveHitTest(const int p_x, const int p_y) const
  {
    return (m_branchAboveRect.Contains(p_x, p_y));
  }
  bool BranchBelowHitTest(const int p_x, const int p_y) const
  {
    return (m_branchBelowRect.Contains(p_x, p_y));
  }

  const wxRect &GetOutcomeExtent() const { return m_outcomeRect; }
  const wxRect &GetPayoffExtent(int pl) const { return m_payoffRect[pl]; }
};

class TreeLayout final : public GameView {
  std::list<std::shared_ptr<NodeEntry>> m_nodeList;
  std::map<GameNode, std::shared_ptr<NodeEntry>> m_nodeMap;

  mutable int m_maxX{0}, m_maxY{0}, m_maxLevel{0};
  int m_infosetSpacing{40};

  const int c_leftMargin{20}, c_topMargin{40}, c_bottomMargin{25};

  std::shared_ptr<NodeEntry> ComputeNextInInfoset(const std::shared_ptr<NodeEntry> &);
  void ComputeSublevel(const std::shared_ptr<NodeEntry> &);

  void BuildNodeList(const GameNode &, const BehaviorSupportProfile &, int);

  /// (Recursively) compute the y-offsets of all nodes
  void ComputeOffsets(const GameNode &, const BehaviorSupportProfile &, int &);
  /// Based on node levels and information set sublevels, compute the depth
  /// (X coordinate) of all nodes
  void ComputeNodeDepths(const Gambit::Layout &) const;
  void ComputeRenderedParents() const;

  wxString CreateNodeLabel(const std::shared_ptr<NodeEntry> &, int) const;
  wxString CreateBranchLabel(const std::shared_ptr<NodeEntry> &, int) const;

  void RenderSubtree(wxDC &dc, bool p_noHints) const;

  // Overriding GameView members
  void OnUpdate() override {}

  void DrawNode(wxDC &, const std::shared_ptr<NodeEntry> &, const GameNode &selection,
                bool p_noHints) const;
  void DrawIncomingBranch(wxDC &, const std::shared_ptr<NodeEntry> &) const;
  void DrawOutcome(wxDC &, const std::shared_ptr<NodeEntry> &, bool p_noHints) const;

  bool NodeHitTest(const std::shared_ptr<NodeEntry> &p_entry, int p_x, int p_y) const;

public:
  explicit TreeLayout(GameDocument *p_doc) : GameView(p_doc) {}
  ~TreeLayout() override = default;

  GameNode PriorSameLevel(const GameNode &) const;
  GameNode NextSameLevel(const GameNode &) const;

  void BuildNodeList(const BehaviorSupportProfile &);
  void Layout(const BehaviorSupportProfile &);
  void GenerateLabels() const;

  std::shared_ptr<NodeEntry> GetNodeEntry(const GameNode &p_node) const
  {
    return m_nodeMap.at(p_node);
  }
  /// Return the layout entry for the most immediate predecessor of p_node
  /// which is rendered in the layout
  std::shared_ptr<NodeEntry> GetRenderedAncestor(const GameNode &p_node) const;
  /// Return the layout entry for the first descendant node of p_node
  /// which is rendered in the layout, as determined by the depth-first traversal.
  std::shared_ptr<NodeEntry> GetRenderedDescendant(const GameNode &p_node) const;

  int MaxX() const { return m_maxX; }
  int MaxY() const { return m_maxY; }

  GameNode NodeHitTest(int, int) const;
  GameNode OutcomeHitTest(int, int) const;
  GameNode BranchAboveHitTest(int, int) const;
  GameNode BranchBelowHitTest(int, int) const;
  bool InfosetHitTest(const std::shared_ptr<NodeEntry> &p_entry, int p_x, int p_y) const;
  GameNode InfosetHitTest(int, int) const;

  void Render(wxDC &, bool p_noHints) const;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_EFGLAYOUT_H
