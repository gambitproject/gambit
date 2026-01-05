//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/gametree.h
// Declaration of extensive game representation
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

#ifndef GAMETREE_H
#define GAMETREE_H

#include "gameexpl.h"

namespace Gambit {

class GameTreeRep final : public GameExplicitRep {
  friend class GameNodeRep;
  friend class GameInfosetRep;
  friend class GameActionRep;

  struct OwnPriorActionInfo {
    std::map<GameNodeRep *, GameActionRep *> node_map;
    std::map<GameInfosetRep *, std::set<GameActionRep *>> infoset_map;
  };

protected:
  mutable bool m_computedValues{false}, m_nodesOrdered{false}, m_infosetsOrdered{false};
  std::shared_ptr<GameNodeRep> m_root;
  std::shared_ptr<GamePlayerRep> m_chance;
  std::size_t m_numNodes = 1;
  std::size_t m_numNonterminalNodes = 0;
  std::map<GameNodeRep *, std::vector<GameNodeRep *>> m_nodePlays;
  mutable std::shared_ptr<OwnPriorActionInfo> m_ownPriorActionInfo;
  mutable std::unique_ptr<std::set<GameNodeRep *>> m_unreachableNodes;
  mutable std::set<GameInfosetRep *> m_absentMindedInfosets;

  /// @name Private auxiliary functions
  //@{
  static void SortInfosets(GamePlayerRep *);
  static void RenumberInfosets(GamePlayerRep *);
  /// Normalize the probability distribution of actions at a chance node
  Game NormalizeChanceProbs(GameInfosetRep *);
  //@}

  /// @name Managing the representation
  //@{
  void InvalidateNodeOrdering() const
  {
    m_nodesOrdered = false;
    m_infosetsOrdered = false;
  }
  void InvalidateInfosetOrdering() const { m_infosetsOrdered = false; }
  void EnsureNodeOrdering() const override;
  void EnsureInfosetOrdering() const override;

  void BuildComputedValues() const override;
  void BuildConsistentPlays();
  void ClearComputedValues() const;

  /// Removes the node from the information set, invalidating if emptied
  void RemoveMember(GameInfosetRep *, GameNodeRep *);

  void CopySubtree(GameNodeRep *, GameNodeRep *, GameNodeRep *);
  //@}

public:
  /// @name Lifecycle
  //@{
  GameTreeRep();
  ~GameTreeRep() override;
  Game Copy() const override;
  //@}

  /// @name General data access
  //@{
  bool IsTree() const override { return true; }
  bool IsConstSum() const override;
  bool IsPerfectRecall() const override;

  /// Returns the smallest payoff to the player in any play of the game
  Rational GetPlayerMinPayoff(const GamePlayer &) const override;
  /// Returns the largest payoff to the player in any play of the game
  Rational GetPlayerMaxPayoff(const GamePlayer &) const override;
  bool IsAbsentMinded(const GameInfoset &p_infoset) const override;
  //@}

  /// @name Players
  //@{
  /// Returns the chance (nature) player
  GamePlayer GetChance() const override { return m_chance->shared_from_this(); }
  /// Creates a new player in the game, with no moves
  GamePlayer NewPlayer() override;
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  GameNode GetRoot() const override { return m_root; }
  /// Returns the number of nodes in the game
  size_t NumNodes() const override { return m_numNodes; }
  /// Returns the number of non-terminal nodes in the game
  size_t NumNonterminalNodes() const override { return m_numNonterminalNodes; }
  /// Returns the last action taken by the node's owner before reaching this node
  GameAction GetOwnPriorAction(const GameNode &p_node) const override;
  //@}

  void DeleteOutcome(const GameOutcome &) override;

  /// @name Writing data files
  //@{
  void WriteEfgFile(std::ostream &, const GameNode &p_node = nullptr) const override;
  void WriteNfgFile(std::ostream &) const override;
  //@}

  /// @name Dimensions of the game
  //@{
  /// Returns the total number of actions in the game
  int BehavProfileLength() const override;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  GameInfoset GetInfoset(int iset) const override;
  /// Returns the set of actions taken by the infoset's owner before reaching this infoset
  std::set<GameAction> GetOwnPriorActions(const GameInfoset &p_infoset) const override;
  //@}

  /// @name Modification
  //@{
  GameInfoset AppendMove(GameNode p_node, GamePlayer p_player, int p_actions,
                         bool p_generateLabels = false) override;
  GameInfoset AppendMove(GameNode p_node, GameInfoset p_infoset) override;
  GameInfoset InsertMove(GameNode p_node, GamePlayer p_player, int p_actions,
                         bool p_generateLabels = false) override;
  GameInfoset InsertMove(GameNode p_node, GameInfoset p_infoset) override;
  void CopyTree(GameNode dest, GameNode src) override;
  void MoveTree(GameNode dest, GameNode src) override;
  void DeleteParent(GameNode) override;
  void DeleteTree(GameNode) override;
  void SetPlayer(GameInfoset, GamePlayer) override;
  void Reveal(GameInfoset, GamePlayer) override;
  void SetInfoset(GameNode, GameInfoset) override;
  GameInfoset LeaveInfoset(GameNode) override;
  Game SetChanceProbs(const GameInfoset &, const Array<Number> &) override;
  GameAction InsertAction(GameInfoset, GameAction p_where = nullptr) override;
  void DeleteAction(GameAction) override;
  void SetOutcome(const GameNode &p_node, const GameOutcome &p_outcome) override;

  std::vector<GameNode> GetPlays(GameNode node) const override;
  std::vector<GameNode> GetPlays(GameInfoset infoset) const override;
  std::vector<GameNode> GetPlays(GameAction action) const override;

  Game CopySubgame(GameNode) const override;
  //@}

  PureStrategyProfile NewPureStrategyProfile() const override;
  MixedStrategyProfile<double> NewMixedStrategyProfile(double) const override;
  MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const override;
  MixedStrategyProfile<double>
  NewMixedStrategyProfile(double, const StrategySupportProfile &) const override;
  MixedStrategyProfile<Rational>
  NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &) const override;

private:
  std::vector<GameNodeRep *> BuildConsistentPlaysRecursiveImpl(GameNodeRep *node);
  void BuildOwnPriorActions() const;
  void BuildUnreachableNodes() const;
};

template <class T> class TreeMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {
public:
  explicit TreeMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  {
  }
  explicit TreeMixedStrategyProfileRep(const MixedBehaviorProfile<T> &);
  ~TreeMixedStrategyProfileRep() override = default;

  std::unique_ptr<MixedStrategyProfileRep<T>> Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;

private:
  mutable std::shared_ptr<MixedBehaviorProfile<T>> m_mixedBehavior;

  void MakeBehavior() const;
  void OnProfileChanged() const override;
};

} // namespace Gambit

#endif // GAMETREE_H
