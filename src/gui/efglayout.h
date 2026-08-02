//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

#include "games/layout.h"

namespace Gambit::GUI {

/// All of a rendered node's screen coordinates in one place: its position
/// (set by the layout pass -- TreeLayout::ComputeNodeDepths/Layout() -- not
/// by ComputeGeometry() below), its own token (the shape drawn there -- a
/// circle, box, diamond, dot, or horizontal line, per style), the outcome
/// label (or "no outcome" hint), one payoff cell per player (1-indexed,
/// matching player numbers), and the two incoming-branch labels. Everything
/// but position is computed once, up front, by TreeLayout::ComputeGeometry()
/// -- not as a side effect of painting -- so that hit-testing is always
/// valid without requiring a prior paint event.
struct NodeGeometry {
  wxPoint position;
  wxRect token;
  wxRect outcome;
  Array<wxRect> payoffs;
  wxRect branchAbove, branchBelow;
};

class NodeEntry {
  friend class TreeLayout;
  GameNode m_node;                     // the corresponding node in the game
  std::shared_ptr<NodeEntry> m_parent; // parent node
  NodeGeometry m_geometry;

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

  int GetX() const { return m_geometry.position.x; }
  void SetX(int p_x) { m_geometry.position.x = p_x; }
  int GetY() const { return m_geometry.position.y; }
  void SetY(int p_y) { m_geometry.position.y = p_y; }

  int GetChildNumber() const
  {
    return (m_node->GetParent()) ? m_node->GetPriorAction()->GetNumber() : 0;
  }

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

  bool NodeHitTest(const int p_x, const int p_y) const
  {
    return (m_geometry.token.Contains(p_x, p_y));
  }
  bool OutcomeHitTest(const int p_x, const int p_y) const
  {
    return (m_geometry.outcome.Contains(p_x, p_y));
  }
  bool BranchAboveHitTest(const int p_x, const int p_y) const
  {
    return (m_geometry.branchAbove.Contains(p_x, p_y));
  }
  bool BranchBelowHitTest(const int p_x, const int p_y) const
  {
    return (m_geometry.branchBelow.Contains(p_x, p_y));
  }

  const wxRect &GetOutcomeExtent() const { return m_geometry.outcome; }
  const wxRect &GetPayoffExtent(int pl) const { return m_geometry.payoffs[pl]; }
};

/// What was hit by a point query against the rendered tree, and which node it
/// belongs to. BranchAbove/BranchBelow report the CHILD node whose incoming
/// branch was hit (not its parent) -- callers wanting the parent (e.g. to
/// select the node whose move is being edited) call node->GetParent().
/// Payoff reports which player's cell (1-indexed) within the outcome region
/// was hit; Outcome covers the rest of the outcome region (including the
/// "no outcome" hint).
enum class HitRegion { None, Node, Outcome, Payoff, BranchAbove, BranchBelow };

struct HitResult {
  HitRegion region{HitRegion::None};
  GameNode node;
  int payoffPlayer{0}; // valid only when region == HitRegion::Payoff
  explicit operator bool() const { return region != HitRegion::None; }
};

class TreeLayout final : public GameView {
  std::list<std::shared_ptr<NodeEntry>> m_nodeList;
  std::map<GameNode, std::shared_ptr<NodeEntry>> m_nodeMap;

  mutable int m_maxX{0}, m_maxY{0};
  int m_infosetSpacing{40};

  const int c_leftMargin{20}, c_topMargin{40}, c_bottomMargin{25};

  std::shared_ptr<NodeEntry> ComputeNextInInfoset(const std::shared_ptr<NodeEntry> &) const;

  /// Creates the NodeEntry for p_node (and recursively, its descendants),
  /// linking each to p_parentEntry (nullptr for the root).
  void BuildNodeList(const GameNode &p_node, const std::shared_ptr<NodeEntry> &p_parentEntry);

  /// Based on node levels and information set sublevels, compute the depth
  /// (X coordinate) of all nodes. The actual placement strategy is the free
  /// function ComputeLevelProportionalX in efglayout.cc; this just supplies
  /// it with this instance's style/spacing parameters.
  void ComputeNodeDepths(const Layout &) const;

  /// Computes NodeGeometry (token/outcome/payoff/branch-label regions) for
  /// every node, and grows m_maxX to account for outcome-label width, all
  /// without painting anything.  Must run after GenerateLabels() (it measures
  /// label text); relies on each entry's m_parent, set once by BuildNodeList.
  void ComputeGeometry() const;
  /// Computes the region occupied by the node's own token (its shape, per
  /// style -- see GetTokenForNode in efglayout.cc), independent of any text
  /// measurement, so unlike its siblings below this needs no wxDC.
  void ComputeTokenGeometry(const std::shared_ptr<NodeEntry> &) const;
  void ComputeBranchGeometry(wxDC &, const std::shared_ptr<NodeEntry> &) const;
  void ComputeOutcomeGeometry(wxDC &, const std::shared_ptr<NodeEntry> &) const;

  void RenderSubtree(wxDC &dc, bool p_noHints) const;

  // Overriding GameView members
  void OnUpdate() override {}

  void DrawNode(wxDC &, const std::shared_ptr<NodeEntry> &, const GameNode &selection,
                bool p_noHints) const;
  void DrawIncomingBranch(wxDC &, const std::shared_ptr<NodeEntry> &) const;
  void DrawOutcome(wxDC &, const std::shared_ptr<NodeEntry> &, bool p_noHints) const;

  void BuildNodeList(const Game &);

public:
  explicit TreeLayout(GameDocument *p_doc) : GameView(p_doc) {}
  ~TreeLayout() override = default;

  GameNode PriorSameLevel(const GameNode &) const;
  GameNode NextSameLevel(const GameNode &) const;

  void Layout(const Game &);
  void GenerateLabels() const;

  std::shared_ptr<NodeEntry> GetNodeEntry(const GameNode &p_node) const
  {
    return m_nodeMap.at(p_node);
  }

  int MaxX() const { return m_maxX; }
  int MaxY() const { return m_maxY; }

  GameNode NodeHitTest(int, int) const;
  HitResult HitTest(int, int) const;

  void Render(wxDC &, bool p_noHints) const;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_EFGLAYOUT_H
