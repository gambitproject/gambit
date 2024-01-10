//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include <list>
#include <map>
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

  std::map<GameInfoset, bool> m_infosetReachable;
  std::map<GameNode, bool> m_nonterminalReachable;

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
  /// Returns the total number of actions in the support
  size_t BehaviorProfileLength() const;

  /// Returns the action at the specified position in the support
  GameAction GetAction(int pl, int iset, int act) const
  { return m_actions[pl][iset][act]; }
  /// Returns the set of actions in the support at the information set
  const Array<GameAction> &GetActions(const GameInfoset &p_infoset) const
  { return m_actions[p_infoset->GetPlayer()->GetNumber()][p_infoset->GetNumber()]; }
  /// Does the information set have at least one active action?
  bool HasAction(const GameInfoset &p_infoset) const
  { return !m_actions[p_infoset->GetPlayer()->GetNumber()][p_infoset->GetNumber()].empty(); }

  /// Returns the position of the action in the support. 
  int GetIndex(const GameAction &) const;

  /// Returns whether the action is in the support.
  bool Contains(const GameAction &p_action) const
  { return (GetIndex(p_action) != 0); }
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
  /// Can the information set be reached under this support?
  bool IsReachable(const GameInfoset &p_infoset) const
  { return m_infosetReachable.at(p_infoset); }
  /// Get the information sets for the player reachable under the support
  std::list<GameInfoset> GetInfosets(const GamePlayer &) const;
  /// Get the members of the information set reachable under the support
  std::list<GameNode> GetMembers(const GameInfoset &) const;
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









