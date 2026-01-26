//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
  Game m_game;
  CartesianSubset m_strategyDigits;

public:
  class Support {
    const StrategySupportProfile *m_profile;
    size_t m_playerIndex;

  public:
    class const_iterator {
      const StrategySupportProfile *m_profile{nullptr};
      size_t m_playerIndex{0};
      std::vector<int>::const_iterator m_it;

    public:
      using value_type = GameStrategy;
      using reference = GameStrategy;
      using pointer = void;
      using difference_type = std::ptrdiff_t;
      using iterator_category = std::forward_iterator_tag;

      const_iterator() = default;
      const_iterator(const StrategySupportProfile *profile, const size_t playerIndex,
                     const std::vector<int>::const_iterator it)
        : m_profile(profile), m_playerIndex(playerIndex), m_it(it)
      {
      }

      GameStrategy operator*() const
      {
        const auto &player = m_profile->m_game->GetPlayer(m_playerIndex + 1);
        return player->GetStrategy(*m_it + 1);
      }

      const_iterator &operator++()
      {
        ++m_it;
        return *this;
      }

      bool operator==(const const_iterator &other) const { return m_it == other.m_it; }

      bool operator!=(const const_iterator &other) const { return !(*this == other); }
    };

    Support() : m_profile(nullptr), m_playerIndex(0) {}

    Support(const StrategySupportProfile *profile, GamePlayer player)
      : m_profile(profile), m_playerIndex(player->GetNumber() - 1)
    {
    }

    size_t size() const
    {
      return m_profile->m_strategyDigits.m_allowedDigits[m_playerIndex].size();
    }

    GameStrategy operator[](const size_t index) const
    {
      const int digit = m_profile->m_strategyDigits.m_allowedDigits[m_playerIndex][index];
      return m_profile->m_game->GetPlayer(m_playerIndex + 1)->GetStrategy(digit + 1);
    }

    GameStrategy front() const
    {
      const int digit = m_profile->m_strategyDigits.m_allowedDigits[m_playerIndex].front();
      return m_profile->m_game->GetPlayer(m_playerIndex + 1)->GetStrategy(digit + 1);
    }

    GameStrategy back() const
    {
      const int digit = m_profile->m_strategyDigits.m_allowedDigits[m_playerIndex].back();
      return m_profile->m_game->GetPlayer(m_playerIndex + 1)->GetStrategy(digit + 1);
    }

    const_iterator begin() const
    {
      const auto &digits = m_profile->m_strategyDigits.m_allowedDigits[m_playerIndex];
      return {m_profile, m_playerIndex, digits.begin()};
    }

    const_iterator end() const
    {
      const auto &digits = m_profile->m_strategyDigits.m_allowedDigits[m_playerIndex];
      return {m_profile, m_playerIndex, digits.end()};
    }
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
    return m_game == p_support.m_game &&
           m_strategyDigits.m_allowedDigits == p_support.m_strategyDigits.m_allowedDigits;
  }
  /// Test for the inequality of two supports
  bool operator!=(const StrategySupportProfile &p_support) const
  {
    return m_game != p_support.m_game ||
           m_strategyDigits.m_allowedDigits != p_support.m_strategyDigits.m_allowedDigits;
  }
  //@}

  /// @name General information
  //@{
  /// Returns the game on which the support is defined.
  Game GetGame() const { return m_game; }

  /// Returns the total number of strategies in the support.
  int MixedProfileLength() const;

  template <class T> MixedStrategyProfile<T> NewMixedStrategyProfile() const;

  /// Returns the number of players in the game
  int NumPlayers() const { return m_game->NumPlayers(); }
  /// Returns the set of players in the game
  GameRep::Players GetPlayers() const { return m_game->GetPlayers(); }
  /// Returns the set of strategies in the support for a player
  Support GetStrategies(const GamePlayer &p_player) const { return {this, p_player}; }

  /// Returns true exactly when the strategy is in the support.
  bool Contains(const GameStrategy &s) const
  {
    const auto &digits = m_strategyDigits.m_allowedDigits[s->GetPlayer()->GetNumber() - 1];
    const int digit = s->GetNumber() - 1;
    return std::binary_search(digits.begin(), digits.end(), digit);
  }

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

  StrategySupportProfile RestrictTo(const GameStrategy &p_strategy) const
  {
    if (p_strategy->GetGame() != m_game) {
      throw MismatchException();
    }
    const GamePlayer player = p_strategy->GetPlayer();
    const size_t player_index = player->GetNumber() - 1;
    const int digit = p_strategy->GetNumber() - 1;
    StrategySupportProfile restricted(*this);
    restricted.m_strategyDigits.m_allowedDigits[player_index].assign(1, digit);
    return restricted;
  }
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
