//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/behavpure.h
// Declaration of pure behavior profile
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

#ifndef GAMBIT_GAMES_BEHAVPURE_H
#define GAMBIT_GAMES_BEHAVPURE_H

#include "behavspt.h"

namespace Gambit {

/// This class represents a behavior profile on an extensive game.
/// It specifies exactly one strategy for each information set in the
/// game.
class PureBehaviorProfile {
private:
  Game m_efg;
  std::map<GameInfoset, GameAction> m_profile;

public:
  /// @name Lifecycle
  //@{
  /// Construct a new behavior profile on the specified game
  explicit PureBehaviorProfile(const Game &);
  //@}

  bool operator==(const PureBehaviorProfile &p_other) const
  {
    return m_profile == p_other.m_profile;
  }

  /// @name Data access and manipulation
  //@{
  /// Get the action played at an information set
  GameAction GetAction(const GameInfoset &p_infoset) const { return m_profile.at(p_infoset); }

  /// Set the action played at an information set
  void SetAction(const GameAction &p_action) { m_profile[p_action->GetInfoset()] = p_action; }

  /// Get the payoff to the player that results from the profile
  template <class T> T GetPayoff(const GamePlayer &p_player) const;
  /// Get the payoff to the player, conditional on reaching a node
  template <class T> T GetPayoff(const GameNode &, const GamePlayer &) const;
  /// Get the payoff to playing the action, conditional on the profile
  template <class T> T GetPayoff(const GameAction &) const;

  /// Is the profile a pure strategy agent Nash equilibrium?
  bool IsAgentNash() const;

  /// Convert to a mixed behavior representation
  MixedBehaviorProfile<Rational> ToMixedBehaviorProfile() const;
  //@}
};

template <> inline double PureBehaviorProfile::GetPayoff(const GamePlayer &p_player) const
{
  return GetPayoff<double>(m_efg->GetRoot(), p_player);
}

template <> inline Rational PureBehaviorProfile::GetPayoff(const GamePlayer &p_player) const
{
  return GetPayoff<Rational>(m_efg->GetRoot(), p_player);
}

template <> inline std::string PureBehaviorProfile::GetPayoff(const GamePlayer &p_player) const
{
  return lexical_cast<std::string>(GetPayoff<Rational>(m_efg->GetRoot(), p_player));
}

class BehaviorContingencies {
private:
  BehaviorSupportProfile m_support;
  std::vector<GameAction> m_frozen;
  std::list<GameInfoset> m_activeInfosets;

public:
  class iterator {
  private:
    BehaviorContingencies *m_cont;
    bool m_atEnd;
    std::map<GameInfoset, BehaviorSupportProfile::Support::const_iterator> m_currentBehav;
    PureBehaviorProfile m_profile;

  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = PureBehaviorProfile;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(BehaviorContingencies *, bool p_end);

    iterator &operator++();

    bool operator==(const iterator &p_other) const
    {
      if (m_atEnd && p_other.m_atEnd && m_cont == p_other.m_cont) {
        return true;
      }
      if (m_atEnd != p_other.m_atEnd || m_cont != p_other.m_cont) {
        return false;
      }
      return (m_profile == p_other.m_profile);
    }
    bool operator!=(const iterator &p_other) const { return !(*this == p_other); }

    PureBehaviorProfile &operator*() { return m_profile; }
    const PureBehaviorProfile &operator*() const { return m_profile; }
  };
  /// @name Lifecycle
  //@{
  /// Construct a new iterator on the support, holding the listed actions fixed
  explicit BehaviorContingencies(const BehaviorSupportProfile &,
                                 const std::set<GameInfoset> &p_active = {},
                                 const std::vector<GameAction> &p_frozen = {});
  //@}
  iterator begin() { return {this, false}; }
  iterator end() { return {this, true}; }
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_BEHAVPURE_H
