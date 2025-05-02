//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

class GameTreeRep : public GameExplicitRep {
  friend class GameNodeRep;
  friend class GameInfosetRep;
  friend class GameActionRep;

protected:
  mutable bool m_computedValues{false}, m_doCanon{true};
  GameNodeRep *m_root;
  GamePlayerRep *m_chance;
  std::size_t m_numNodes = 1;
  std::size_t m_numNonterminalNodes = 0;

  /// @name Private auxiliary functions
  //@{
  void NumberNodes(GameNodeRep *, int &);
  /// Normalize the probability distribution of actions at a chance node
  Game NormalizeChanceProbs(const GameInfoset &);
  //@}

  /// @name Managing the representation
  //@{
  void Canonicalize();
  void BuildComputedValues() const override;
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
  using GameRep::IsPerfectRecall;
  bool IsPerfectRecall(GameInfoset &, GameInfoset &) const override;
  /// Turn on or off automatic canonicalization of the game
  void SetCanonicalization(bool p_doCanon) const
  {
    m_doCanon = p_doCanon;
    if (m_doCanon) {
      const_cast<GameTreeRep *>(this)->Canonicalize();
    }
  }
  //@}

  /// @name Players
  //@{
  /// Returns the chance (nature) player
  GamePlayer GetChance() const override { return m_chance; }
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
  /// Returns the set of information sets in the game
  Array<GameInfoset> GetInfosets() const override;
  /// Returns an array with the number of information sets per personal player
  Array<int> NumInfosets() const override;
  //@}

  /// @name Modification
  //@{
  GameInfoset AppendMove(GameNode p_node, GamePlayer p_player, int p_actions) override;
  GameInfoset AppendMove(GameNode p_node, GameInfoset p_infoset) override;
  GameInfoset InsertMove(GameNode p_node, GamePlayer p_player, int p_actions) override;
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
  void SetOutcome(GameNode, const GameOutcome &p_outcome) override;

  Game CopySubgame(GameNode) const override;
  //@}

  PureStrategyProfile NewPureStrategyProfile() const override;
  MixedStrategyProfile<double> NewMixedStrategyProfile(double) const override;
  MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const override;
  MixedStrategyProfile<double>
  NewMixedStrategyProfile(double, const StrategySupportProfile &) const override;
  MixedStrategyProfile<Rational>
  NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &) const override;
};

template <class T> class TreeMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {
public:
  explicit TreeMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  {
  }
  explicit TreeMixedStrategyProfileRep(const MixedBehaviorProfile<T> &);
  ~TreeMixedStrategyProfileRep() override = default;

  MixedStrategyProfileRep<T> *Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;

private:
  mutable std::shared_ptr<MixedBehaviorProfile<T>> mixed_behav_profile_sptr;

  void MakeBehavior() const;
  void InvalidateCache() const override;
};

} // namespace Gambit

#endif // GAMETREE_H
