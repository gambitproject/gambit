//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/stratspt.h
// Interface to strategy classes for normal forms
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

#ifndef LIBGAMBIT_STRATSPT_H
#define LIBGAMBIT_STRATSPT_H

#include "gambit.h"

namespace Gambit {

/// @brief A support on a strategic game
///
/// This class represents a subset of the strategies in strategic game.
/// It is enforced that each player has at least one strategy; thus,
/// the strategies in a support can be viewed as a restriction of a game
/// to a subset of its strategies.  This is useful for eliminating
/// dominated strategies from consideration, and in computational
/// approaches that enumerate possible equilibrium supports.
///
/// Within the support, strategies are maintained in the same order
/// in which they appear in the underlying game.
class StrategySupportProfile {
protected:
  Game m_nfg;
  std::map<GamePlayer, std::vector<GameStrategy>> m_support;

public:
  class Support {
  private:
    const StrategySupportProfile *m_profile;
    GamePlayer m_player;

  public:
    using const_iterator = std::vector<GameStrategy>::const_iterator;

    Support() : m_profile(nullptr), m_player(nullptr) {}
    Support(const StrategySupportProfile *p_profile, GamePlayer p_player)
      : m_profile(p_profile), m_player(p_player)
    {
    }

    size_t size() const { return m_profile->m_support.at(m_player).size(); }
    GameStrategy front() const { return m_profile->m_support.at(m_player).front(); }
    GameStrategy back() const { return m_profile->m_support.at(m_player).back(); }

    const_iterator begin() const { return m_profile->m_support.at(m_player).begin(); }
    const_iterator end() const { return m_profile->m_support.at(m_player).end(); }
  };

  /// @name Lifecycle
  //@{
  /// Constructor.  By default, a support contains all strategies.
  StrategySupportProfile(const Game &);
  //@}

  /// @name Operator overloading
  //@{
  /// Test for the equality of two supports (same strategies for all players)
  bool operator==(const StrategySupportProfile &p_support) const
  {
    return (m_support == p_support.m_support);
  }
  /// Test for the inequality of two supports
  bool operator!=(const StrategySupportProfile &p_support) const
  {
    return (m_support != p_support.m_support);
  }
  //@}

  /// @name General information
  //@{
  /// Returns the game on which the support is defined.
  Game GetGame() const { return m_nfg; }

  /// Returns the number of strategies in the support for all players.
  Array<int> NumStrategies() const;

  /// Returns the total number of strategies in the support.
  int MixedProfileLength() const;

  template <class T> MixedStrategyProfile<T> NewMixedStrategyProfile() const;

  /// Returns the number of players in the game
  int NumPlayers() const { return m_nfg->NumPlayers(); }
  /// Returns the set of players in the game
  Array<GamePlayer> GetPlayers() const { return m_nfg->GetPlayers(); }
  /// Returns the set of strategies in the support for a player
  Support GetStrategies(const GamePlayer &p_player) const { return Support(this, p_player); }

  /// Returns true exactly when the strategy is in the support.
  bool Contains(const GameStrategy &s) const { return contains(m_support.at(s->GetPlayer()), s); }

  /// Returns true iff this support is a (weak) subset of the specified support
  bool IsSubsetOf(const StrategySupportProfile &) const;

  //@}

  /// @name Writing data files
  //@{
  void WriteNfgFile(std::ostream &p_file) const;
  //@}

  /// @name Modifying the support
  //@{
  /// Add a strategy to the support.
  void AddStrategy(const GameStrategy &);

  /// @brief Removes a strategy from the support
  ///
  /// Removes a strategy from the support.  If the strategy is
  /// not present, or if the strategy is the only strategy for that
  /// player, it is not removed.  Returns true if the removal was
  /// executed, and false if not.
  bool RemoveStrategy(const GameStrategy &);
  //@}

  /// @name Identification of dominated strategies
  //@{
  bool Dominates(const GameStrategy &s, const GameStrategy &t, bool p_strict) const;
  bool IsDominated(const GameStrategy &s, bool p_strict, bool p_external = false) const;

  /// Returns a copy of the support with dominated strategies removed
  StrategySupportProfile Undominated(bool p_strict, bool p_external = false) const;
  //@}

  /// @name Identification of overwhelmed strategies
  //@{
  bool Overwhelms(const GameStrategy &s, const GameStrategy &t, bool p_strict) const;
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_STRATSPT_H
