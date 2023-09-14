//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/behavspt.h
// Interface to supports for extensive forms
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

#ifndef LIBGAMBIT_BEHAVSPT_H
#define LIBGAMBIT_BEHAVSPT_H

#include "game.h"

namespace Gambit {

/// This class represents a subset of the actions in an extensive game.
/// It is enforced that each player has at least one action at each
/// information set; thus, the actions in a support can be viewed as
/// a restriction of a game to a subset of its actions.  This is useful
/// for eliminating dominated strategies from consideration, and in
/// computational approaches that enumerate possible equilibrium
/// supports.
class BehaviorSupportProfile {
protected:
  Game m_efg;
  Array<Array<Array<GameAction> > > m_actions;

  Array<List<bool> > m_infosetActive;
  Array<List<List<bool> > > m_nonterminalActive;

  void activate(const GameNode &);
  void deactivate(const GameNode &);
  void activate(const GameInfoset &);
  void deactivate(const GameInfoset &);
  bool HasActiveMembers(int pl, int iset) const;
  void ActivateSubtree(const GameNode &);
  void DeactivateSubtree(const GameNode &);
  void DeactivateSubtree(const GameNode &, List<GameInfoset> &);

public:
  /// @name Lifecycle
  //@{
  /// Constructor.  By default, a support contains all strategies. 
  explicit BehaviorSupportProfile(const Game &);
  ~BehaviorSupportProfile() = default;

  //@}

  /// @name Operator overloading
  //@{
  /// Test for the equality of two supports (same actions at all infosets)
  bool operator==(const BehaviorSupportProfile &) const;
  bool operator!=(const BehaviorSupportProfile &p_support) const
  { return !(*this == p_support); }

  /// @name General information
  //@{
  /// Returns the game on which the support is defined.
  Game GetGame() const { return m_efg; }

  /// Returns the number of actions in the information set
  int NumActions(const GameInfoset &p_infoset) const
  { return m_actions[p_infoset->GetPlayer()->GetNumber()][p_infoset->GetNumber()].Length(); }
  int NumActions(int pl, int iset) const
  { return m_actions[pl][iset].Length(); }

  /// Returns the number of actions in the support for all information sets
  PVector<int> NumActions() const;

  /// Returns the action at the specified position in the support
  GameAction GetAction(const GameInfoset &p_infoset, int p_act) const
  { return m_actions[p_infoset->GetPlayer()->GetNumber()][p_infoset->GetNumber()][p_act]; }
  GameAction GetAction(int pl, int iset, int act) const
  { return m_actions[pl][iset][act]; }

  /// Returns the position of the action in the support. 
  int GetIndex(const GameAction &) const;

  /// Returns whether the action is in the support.
  bool Contains(const GameAction &p_action) const
  { return (GetIndex(p_action) != 0); }

  /// The dimension of a behavior strategy at reachable information sets
  int NumDegreesOfFreedom() const;

  /// Does the information set have at least one active action?
  bool HasActiveActionAt(const GameInfoset &) const;
  /// Do all information sets have at least one active action?
  bool HasActiveActionsAtAllInfosets() const;

  /// Total number of sequences 
  int NumSequences() const;
  /// Number of sequences for a player
  int NumSequences(int pl) const;

  /// Is the information set active (i.e., reachable)?
  bool IsActive(const GameInfoset &) const;
  /// How many active members are there in the information set?
  int NumActiveMembers(const GameInfoset &) const;
  /// Is the node active (i.e., reachable)?
  bool IsActive(const GameNode &) const;

  /// Do all active information sets have actions in the support?
  bool HasActiveActionsAtActiveInfosets() const;
  /// Do only active information sets have actions in the support?
  bool HasActiveActionsAtActiveInfosetsAndNoOthers() const;
  //@}

  /// @name Editing the support
  //@{
  /// Adds the action to the support; no effect if action is present already
  void AddAction(const GameAction &);
  /// Removes the action from the support; returns true if successful.
  bool RemoveAction(const GameAction &);
  /// Removes the action and returns the list of information sets
  /// made unreachable by the action's removal
  bool RemoveAction(const GameAction &, List<GameInfoset> &);
  //@}

  /// @name Reachability of nodes and information sets
  //@{
  List<GameNode> ReachableNonterminalNodes() const;
  List<GameNode> ReachableNonterminalNodes(const GameNode &) const;
  /// Sets p_reachable(pl,iset) to 1 if infoset (pl,iset) reachable after p_node
  void ReachableInfosets(const GameNode &p_node, 
			 PVector<int> &p_reachable) const;
  List<GameInfoset> ReachableInfosets(const GamePlayer &) const;

  bool MayReach(const GameNode &) const;
  bool MayReach(const GameInfoset &) const;

  List<GameNode> ReachableMembers(const GameInfoset &) const;
  //@}

  /// @name Identification of dominated actions
  //@{
  /// Returns true if action a is dominated by action b
  bool Dominates(const GameAction &a, const GameAction &b,
		 bool p_strict, bool p_conditional) const;
  /// Returns true if the action is dominated by some other action
  bool IsDominated(const GameAction &a, 
		   bool p_strict, bool p_conditional) const;
  /// Returns a copy of the support with dominated actions eliminated
  BehaviorSupportProfile Undominated(bool p_strict, bool p_conditional,
			   const Array<int> &players,
			   std::ostream &) const;
  //@}
};

} // end namespace Gambit

#endif  // LIBGAMBIT_BEHAVSPT_H









