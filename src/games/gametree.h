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

#include <algorithm>
#include "gameexpl.h"

namespace Gambit {

class GameTreeRep;

class GameTreeActionRep : public GameActionRep {
  friend class GameTreeRep;
  friend class GameTreeInfosetRep;
  template <class T> friend class MixedBehaviorProfile;

private:
  int m_number;
  std::string m_label;
  GameTreeInfosetRep *m_infoset;

  GameTreeActionRep(int p_number, const std::string &p_label, GameTreeInfosetRep *p_infoset)
    : m_number(p_number), m_label(p_label), m_infoset(p_infoset)
  {
  }
  ~GameTreeActionRep() override = default;

public:
  int GetNumber() const override { return m_number; }
  GameInfoset GetInfoset() const override;

  const std::string &GetLabel() const override { return m_label; }
  void SetLabel(const std::string &p_label) override { m_label = p_label; }

  bool Precedes(const GameNode &) const override;

  void DeleteAction() override;
};

class GameTreeInfosetRep : public GameInfosetRep {
  friend class GameTreeRep;
  friend class GameTreeActionRep;
  friend class GamePlayerRep;
  friend class GameTreeNodeRep;
  template <class T> friend class MixedBehaviorProfile;

protected:
  GameTreeRep *m_efg;
  int m_number;
  std::string m_label;
  GamePlayerRep *m_player;
  Array<GameTreeActionRep *> m_actions;
  std::vector<GameTreeNodeRep *> m_members;
  int flag{0}, whichbranch{0};
  Array<Number> m_probs;

  GameTreeInfosetRep(GameTreeRep *p_efg, int p_number, GamePlayerRep *p_player, int p_actions);
  ~GameTreeInfosetRep() override;

  /// Adds the node to the information set
  void AddMember(GameTreeNodeRep *p_node) { m_members.push_back(p_node); }
  /// Removes the node from the information set, invalidating if emptied
  void RemoveMember(GameTreeNodeRep *);

  void RemoveAction(int which);

  void RenumberActions()
  {
    std::for_each(m_actions.begin(), m_actions.end(),
                  [act = 1](GameTreeActionRep *a) mutable { a->m_number = act++; });
  }

public:
  Game GetGame() const override;
  int GetNumber() const override { return m_number; }

  GamePlayer GetPlayer() const override;
  void SetPlayer(GamePlayer p) override;

  bool IsChanceInfoset() const override;

  void SetLabel(const std::string &p_label) override { m_label = p_label; }
  const std::string &GetLabel() const override { return m_label; }

  GameAction InsertAction(GameAction p_where = nullptr) override;

  /// @name Actions
  //@{
  /// Returns the number of actions available at the information set
  size_t NumActions() const override { return m_actions.size(); }
  /// Returns the p_index'th action at the information set
  GameAction GetAction(int p_index) const override { return m_actions[p_index]; }
  /// Returns the actions available at the information set
  Array<GameAction> GetActions() const override;
  //@}

  size_t NumMembers() const override { return m_members.size(); }
  GameNode GetMember(int p_index) const override;
  Array<GameNode> GetMembers() const override;

  bool Precedes(GameNode) const override;

  const Number &GetActionProb(const GameAction &p_action) const override
  {
    if (p_action->GetInfoset() != GameInfoset(const_cast<GameTreeInfosetRep *>(this))) {
      throw MismatchException();
    }
    return m_probs[p_action->GetNumber()];
  }
  void Reveal(GamePlayer) override;
};

class GameTreeNodeRep : public GameNodeRep {
  friend class GameTreeRep;
  friend class GameTreeActionRep;
  friend class GameTreeInfosetRep;
  friend class GamePlayerRep;
  friend class PureBehaviorProfile;
  template <class T> friend class MixedBehaviorProfile;

protected:
  int m_number{0};
  GameTreeRep *m_efg;
  std::string m_label;
  GameTreeInfosetRep *m_infoset{nullptr};
  GameTreeNodeRep *m_parent;
  GameOutcomeRep *m_outcome{nullptr};
  Array<GameTreeNodeRep *> m_children;
  GameTreeNodeRep *whichbranch{nullptr}, *ptr{nullptr};

  GameTreeNodeRep(GameTreeRep *e, GameTreeNodeRep *p);
  ~GameTreeNodeRep() override;

  void DeleteOutcome(GameOutcomeRep *outc);
  void CopySubtree(GameTreeNodeRep *, GameTreeNodeRep *);

public:
  Game GetGame() const override;

  const std::string &GetLabel() const override { return m_label; }
  void SetLabel(const std::string &p_label) override { m_label = p_label; }

  int GetNumber() const override { return m_number; }
  size_t NumChildren() const override { return m_children.size(); }
  GameNode GetChild(int i) const override { return m_children[i]; }
  GameNode GetChild(const GameAction &p_action) const override
  {
    if (p_action->GetInfoset() != m_infoset) {
      throw MismatchException();
    }
    return m_children[p_action->GetNumber()];
  }
  Array<GameNode> GetChildren() const override;

  GameInfoset GetInfoset() const override { return m_infoset; }
  void SetInfoset(GameInfoset) override;
  GameInfoset LeaveInfoset() override;

  bool IsTerminal() const override { return m_children.empty(); }
  GamePlayer GetPlayer() const override { return (m_infoset) ? m_infoset->GetPlayer() : nullptr; }
  GameAction GetPriorAction() const override; // returns null if root node
  GameNode GetParent() const override { return m_parent; }
  GameNode GetNextSibling() const override;
  GameNode GetPriorSibling() const override;

  GameOutcome GetOutcome() const override { return m_outcome; }
  void SetOutcome(const GameOutcome &p_outcome) override;

  bool IsSuccessorOf(GameNode from) const override;
  bool IsSubgameRoot() const override;

  void DeleteParent() override;
  void DeleteTree() override;

  void CopyTree(GameNode src) override;
  void MoveTree(GameNode src) override;

  Game CopySubgame() const override;

  GameInfoset AppendMove(GamePlayer p_player, int p_actions) override;
  GameInfoset AppendMove(GameInfoset p_infoset) override;
  GameInfoset InsertMove(GamePlayer p_player, int p_actions) override;
  GameInfoset InsertMove(GameInfoset p_infoset) override;
};

class GameTreeRep : public GameExplicitRep {
  friend class GameTreeNodeRep;
  friend class GameTreeInfosetRep;
  friend class GameTreeActionRep;

protected:
  mutable bool m_computedValues{false}, m_doCanon{true};
  GameTreeNodeRep *m_root;
  GamePlayerRep *m_chance;
  std::size_t m_numNodes = 1;

  /// @name Private auxiliary functions
  //@{
  void NumberNodes(GameTreeNodeRep *, int &);
  /// Normalize the probability distribution of actions at a chance node
  Game NormalizeChanceProbs(const GameInfoset &);
  //@}

  /// @name Managing the representation
  //@{
  void Canonicalize();
  void BuildComputedValues() override;
  void ClearComputedValues() const;
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
  Game SetChanceProbs(const GameInfoset &, const Array<Number> &) override;
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
