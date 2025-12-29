//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/stratpure.h
// Declaration of pure strategy profile
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

#ifndef GAMBIT_GAMES_STRATPURE_H
#define GAMBIT_GAMES_STRATPURE_H

#include <memory>

#include "game.h"

namespace Gambit {

/// This class represents a strategy profile on a strategic game.
/// It specifies exactly one strategy for each player defined on the
/// game.
class PureStrategyProfileRep {
  friend class PureStrategyProfile;

protected:
  Game m_game;
  long m_index{0};

  /// Construct a new strategy profile
  explicit PureStrategyProfileRep(const Game &p_game) : m_game(p_game)
  {
    for (const auto &stride : p_game->m_pureStrategies.m_strides) {
      constexpr long digit = 0; // The digit of the first strategy
      m_index += digit * stride;
    }
  }

public:
  virtual ~PureStrategyProfileRep() = default;

  /// Create a copy of the strategy profile.
  virtual std::shared_ptr<PureStrategyProfileRep> Copy() const = 0;

  /// @name Data access and manipulation
  //@{
  const Game &GetGame() const { return m_game; }

  /// Get the strategy played by the player
  GameStrategy GetStrategy(const GamePlayer &p_player) const
  {
    const auto &[m_radices, m_strides] = m_game->m_pureStrategies;
    const size_t index = p_player->m_number - 1;
    const long stride = m_strides[index];
    const long radix = m_radices[index];
    const long digit = (m_index / stride) % radix;
    return p_player->m_strategies[digit];
  }

  /// Set the strategy for a player
  void SetStrategy(const GameStrategy &p_strategy)
  {
    const auto &[m_radices, m_strides] = m_game->m_pureStrategies;
    const size_t index = p_strategy->m_player->m_number - 1;
    const long stride = m_strides[index];
    const long digit_old = (m_index / stride) % m_radices[index];
    const long digit_new = p_strategy->m_number - 1;
    m_index += (digit_new - digit_old) * stride;
  }

  /// Get the outcome that results from the profile
  virtual GameOutcome GetOutcome() const = 0;

  /// Set the outcome that results from the profile
  virtual void SetOutcome(GameOutcome p_outcome) = 0;

  /// Get the payoff to the player resulting from the profile
  virtual Rational GetPayoff(const GamePlayer &p_player) const = 0;

  /// Get the value of playing strategy against the profile
  virtual Rational GetStrategyValue(const GameStrategy &) const = 0;

  /// Convert to a mixed strategy representation
  MixedStrategyProfile<Rational> ToMixedStrategyProfile() const;
  //@}
};

class PureStrategyProfile {
  std::shared_ptr<PureStrategyProfileRep> rep;

public:
  PureStrategyProfile(const PureStrategyProfile &r) : rep(r.rep->Copy()) {}

  explicit PureStrategyProfile(const std::shared_ptr<PureStrategyProfileRep> &p_rep) : rep(p_rep)
  {
  }

  ~PureStrategyProfile() = default;

  PureStrategyProfile &operator=(const PureStrategyProfile &r)
  {
    if (&r != this) {
      rep = r.rep->Copy();
    }
    return *this;
  }

  std::shared_ptr<PureStrategyProfileRep> operator->() const { return rep; }
};

class StrategyContingencies {
  StrategySupportProfile m_support;

public:
  class iterator {
    StrategyContingencies *m_cont;
    bool m_atEnd{false};
    std::vector<size_t> m_pos;
    PureStrategyProfile m_profile;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = PureStrategyProfile;
    using difference_type = std::ptrdiff_t;
    using pointer = PureStrategyProfile *;
    using reference = PureStrategyProfile &;

    iterator(StrategyContingencies *cont, bool p_end)
      : m_cont(cont), m_atEnd(p_end),
        m_profile(cont->m_support.GetGame()->NewPureStrategyProfile())
    {
      if (m_atEnd) {
        return;
      }
      const size_t n = m_cont->m_support.GetPlayers().size();
      m_pos.assign(n, 0);
      for (size_t i = 0; i < n; ++i) {
        const GamePlayer player = m_cont->m_support.GetGame()->GetPlayer(i + 1);
        auto support = m_cont->m_support.GetStrategies(player);
        m_profile->SetStrategy(support[0]);
      }
    }

    iterator &operator++()
    {
      const size_t n = m_pos.size();
      for (size_t i = 0; i < n; ++i) {
        const GamePlayer player = m_cont->m_support.GetGame()->GetPlayer(i + 1);
        auto support = m_cont->m_support.GetStrategies(player);
        ++m_pos[i];
        if (m_pos[i] < support.size()) {
          m_profile->SetStrategy(support[m_pos[i]]);
          return *this;
        }
        m_pos[i] = 0;
        m_profile->SetStrategy(support[0]);
      }

      m_atEnd = true;
      return *this;
    }

    bool operator==(const iterator &other) const
    {
      if (m_atEnd && other.m_atEnd) {
        return m_cont == other.m_cont;
      }
      if (m_atEnd != other.m_atEnd) {
        return false;
      }
      return m_profile.operator->() == other.m_profile.operator->();
    }

    bool operator!=(const iterator &other) const { return !(*this == other); }

    PureStrategyProfile &operator*() { return m_profile; }
    const PureStrategyProfile &operator*() const { return m_profile; }
  };

  explicit StrategyContingencies(const Game &p_game) : m_support(StrategySupportProfile(p_game)) {}
  explicit StrategyContingencies(const StrategySupportProfile &support) : m_support(support) {}

  iterator begin() { return {this, false}; }
  iterator end() { return {this, true}; }
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_STRATPURE_H
