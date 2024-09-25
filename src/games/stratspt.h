//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

class StrategySupportProfile;

/// \brief A support on a strategic game
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
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class MixedStrategyProfileRep;
  template <class T> friend class AGGMixedStrategyProfileRep;
  template <class T> friend class BAGGMixedStrategyProfileRep;

protected:
  Game m_nfg;
  Array<Array<GameStrategy>> m_support;

  /// The index into a strategy profile for a strategy (-1 if not in support)
  Array<int> m_profileIndex;

  bool Undominated(StrategySupportProfile &newS, const GamePlayer &, bool p_strict,
                   bool p_external = false) const;

public:
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

  /// Returns the number of strategies in the support for player pl.
  int NumStrategies(int pl) const { return m_support[pl].size(); }

  /// Returns the number of strategies in the support for all players.
  Array<int> NumStrategies() const;

  /// Returns the total number of strategies in the support.
  int MixedProfileLength() const;

  template <class T> MixedStrategyProfile<T> NewMixedStrategyProfile() const;

  /// Returns the strategy in the st'th position for player pl.
  GameStrategy GetStrategy(int pl, int st) const { return m_support[pl][st]; }

  /// Returns the number of players in the game
  int NumPlayers() const { return m_nfg->NumPlayers(); }
  /// Returns the set of players in the game
  Array<GamePlayer> GetPlayers() const { return m_nfg->GetPlayers(); }
  /// Returns the set of strategies in the support for a player
  const Array<GameStrategy> &GetStrategies(const GamePlayer &p_player) const
  {
    return m_support[p_player->GetNumber()];
  }

  /// Returns true exactly when the strategy is in the support.
  bool Contains(const GameStrategy &s) const { return m_profileIndex[s->GetId()] >= 0; }

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

  /// \brief Removes a strategy from the support
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
  /// Returns a copy of the support with dominated strategies of the specified player removed
  StrategySupportProfile Undominated(bool p_strict, const GamePlayer &p_player) const;
  //@}

  /// @name Identification of overwhelmed strategies
  //@{
  bool Overwhelms(const GameStrategy &s, const GameStrategy &t, bool p_strict) const;
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_STRATSPT_H
