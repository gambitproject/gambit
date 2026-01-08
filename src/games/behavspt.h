//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

class GameSequenceForm;
class SequencesWrapper;
class PlayerSequencesWrapper;
class InfosetsWrapper;
class ContingenciesWrapper;

/// This class represents a subset of the actions in an extensive game.
/// It is enforced that each player has at least one action at each
/// information set; thus, the actions in a support can be viewed as
/// a restriction of a game to a subset of its actions.  This is useful
/// for eliminating dominated strategies from consideration, and in
/// computational approaches that enumerate possible equilibrium
/// supports.
class BehaviorSupportProfile {
  Game m_efg;
  std::map<GameInfoset, std::vector<GameAction>> m_actions;
  mutable std::shared_ptr<GameSequenceForm> m_sequenceForm;
  mutable std::shared_ptr<std::map<GameInfoset, bool>> m_reachableInfosets;

  std::map<GameInfoset, bool> m_infosetReachable;
  std::map<GameNode, bool> m_nonterminalReachable;

  bool HasReachableMembers(const GameInfoset &) const;
  void ActivateSubtree(const GameNode &);
  void DeactivateSubtree(const GameNode &);

public:
  class Support {
    const BehaviorSupportProfile *m_profile;
    GameInfoset m_infoset;

  public:
    using const_iterator = std::vector<GameAction>::const_iterator;

    Support() : m_profile(nullptr), m_infoset(nullptr) {}
    Support(const BehaviorSupportProfile *p_profile, const GameInfoset &p_infoset)
      : m_profile(p_profile), m_infoset(p_infoset)
    {
    }

    size_t size() const { return m_profile->m_actions.at(m_infoset).size(); }
    bool empty() const { return m_profile->m_actions.at(m_infoset).empty(); }
    GameAction front() const { return m_profile->m_actions.at(m_infoset).front(); }
    GameAction back() const { return m_profile->m_actions.at(m_infoset).back(); }

    const_iterator begin() const { return m_profile->m_actions.at(m_infoset).begin(); }
    const_iterator end() const { return m_profile->m_actions.at(m_infoset).end(); }
  };

  /// @name Lifecycle
  //@{
  /// Constructor.  By default, a support contains all strategies.
  explicit BehaviorSupportProfile(const Game &);
  ~BehaviorSupportProfile() = default;

  //@}

  /// @name Operator overloading
  //@{
  /// Test for the equality of two supports (same actions at all infosets)
  bool operator==(const BehaviorSupportProfile &p_support) const
  {
    return m_actions == p_support.m_actions;
  }
  bool operator!=(const BehaviorSupportProfile &p_support) const
  {
    return m_actions != p_support.m_actions;
  }

  /// @name General information
  //@{
  /// Returns the game on which the support is defined.
  Game GetGame() const { return m_efg; }

  /// Returns the total number of actions in the support
  size_t BehaviorProfileLength() const;

  /// Returns the set of actions in the support at the information set
  Support GetActions(const GameInfoset &p_infoset) const { return {this, p_infoset}; }
  /// Does the information set have at least one active action?
  bool HasAction(const GameInfoset &p_infoset) const { return !m_actions.at(p_infoset).empty(); }

  /// Returns whether the action is in the support.
  bool Contains(const GameAction &p_action) const
  {
    return contains(m_actions.at(p_action->GetInfoset()), p_action);
  }
  //@}

  /// @name Editing the support
  //@{
  /// Adds the action to the support; no effect if action is present already
  void AddAction(const GameAction &);
  /// Removes the action from the support; returns true if successful.
  bool RemoveAction(const GameAction &);
  //@}

  /// @name Reachability of nodes and information sets
  //@{
  /// Can the information set be reached under this support?
  bool IsReachable(const GameInfoset &p_infoset) const { return m_infosetReachable.at(p_infoset); }
  /// Get the information sets for the player reachable under the support
  std::list<GameInfoset> GetInfosets(const GamePlayer &) const;
  /// Get the members of the information set reachable under the support
  std::list<GameNode> GetMembers(const GameInfoset &) const;
  //@}

  /// @name Identification of dominated actions
  //@{
  /// Returns true if action 'a' is dominated by action 'b'
  bool Dominates(const GameAction &a, const GameAction &b, bool p_strict) const;
  /// Returns true if the action is dominated by some other action
  bool IsDominated(const GameAction &a, bool p_strict) const;
  /// Returns a copy of the support with dominated actions eliminated
  BehaviorSupportProfile Undominated(bool p_strict) const;
  //@}

  class Infosets {
    const BehaviorSupportProfile *m_support;

  public:
    Infosets(const BehaviorSupportProfile *p_support) : m_support(p_support) {}

    size_t size() const
    {
      auto reachable_infosets = m_support->GetReachableInfosets();
      size_t count = 0;
      for (auto [infoset, is_reachable] : *reachable_infosets) {
        if (is_reachable && !infoset->GetPlayer()->IsChance()) {
          ++count;
        }
      }
      return count;
    }
  };

  class Sequences {
    const BehaviorSupportProfile *m_support;

  public:
    class iterator {
      const std::shared_ptr<GameSequenceForm> m_sfg;
      std::map<GamePlayer, std::vector<GameSequence>>::const_iterator m_currentPlayer;
      std::vector<GameSequence>::const_iterator m_currentSequence;

    public:
      iterator(const std::shared_ptr<GameSequenceForm> p_sfg, bool p_end);

      GameSequence operator*() const { return *m_currentSequence; }
      GameSequence operator->() const { return *m_currentSequence; }

      iterator &operator++();

      bool operator==(const iterator &it) const;
      bool operator!=(const iterator &it) const { return !(*this == it); }
    };

    Sequences(const BehaviorSupportProfile *p_support) : m_support(p_support) {}

    size_t size() const;

    iterator begin() const;
    iterator end() const;
  };

  class PlayerSequences {
    const BehaviorSupportProfile *m_support;
    GamePlayer m_player;

  public:
    PlayerSequences(const BehaviorSupportProfile *p_support, const GamePlayer &p_player)
      : m_support(p_support), m_player(p_player)
    {
    }

    size_t size() const;
    std::vector<GameSequence>::const_iterator begin() const;
    std::vector<GameSequence>::const_iterator end() const;
  };

  class SequenceContingencies {
    const BehaviorSupportProfile *m_support;

  public:
    SequenceContingencies(const BehaviorSupportProfile *p_support) : m_support(p_support) {}

    class iterator {
    private:
      const std::shared_ptr<GameSequenceForm> m_sfg;
      bool m_end{false};
      std::map<GamePlayer, size_t> m_indices;

    public:
      using iterator_category = std::input_iterator_tag;

      iterator(const std::shared_ptr<GameSequenceForm> p_sfg, bool p_end = false);

      std::map<GamePlayer, GameSequence> operator*() const;

      std::map<GamePlayer, GameSequence> operator->() const;

      iterator &operator++();

      bool operator==(const iterator &it) const
      {
        return (m_end == it.m_end && m_sfg == it.m_sfg && m_indices == it.m_indices);
      }
      bool operator!=(const iterator &it) const { return !(*this == it); }
    };

    iterator begin() { return {m_support->GetSequenceForm()}; }
    iterator end() { return {m_support->GetSequenceForm(), true}; }
  };

  std::shared_ptr<GameSequenceForm> GetSequenceForm() const;
  Sequences GetSequences() const;
  PlayerSequences GetSequences(GamePlayer &p_player) const;
  int GetConstraintEntry(const GameInfoset &p_infoset, const GameAction &p_action) const;
  const Rational &GetPayoff(const std::map<GamePlayer, GameSequence> &p_profile,
                            const GamePlayer &p_player) const;
  GameRep::Players GetPlayers() const { return GetGame()->GetPlayers(); }
  MixedBehaviorProfile<double>
  ToMixedBehaviorProfile(const std::map<GameSequence, double> &) const;
  Infosets GetInfosets() const { return {this}; };
  SequenceContingencies GetSequenceContingencies() const;

  void FindReachableInfosets(GameNode p_node) const;
  std::shared_ptr<std::map<GameInfoset, bool>> GetReachableInfosets() const;
};

} // end namespace Gambit

#endif // LIBGAMBIT_BEHAVSPT_H
