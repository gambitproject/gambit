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
  Game m_nfg;
  std::map<GamePlayer, GameStrategy> m_profile;

  /// Construct a new strategy profile
  explicit PureStrategyProfileRep(const Game &p_game);

public:
  virtual ~PureStrategyProfileRep() = default;

  /// Create a copy of the strategy profile.
  virtual std::shared_ptr<PureStrategyProfileRep> Copy() const = 0;

  /// @name Data access and manipulation
  //@{
  const Game &GetGame() const { return m_nfg; }

  /// Get the strategy played by the player
  const GameStrategy &GetStrategy(const GamePlayer &p_player) const
  {
    return m_profile.at(p_player);
  }

  /// Set the strategy for a player
  virtual void SetStrategy(const GameStrategy &p_strategy)
  {
    m_profile[p_strategy->GetPlayer()] = p_strategy;
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
private:
  std::shared_ptr<PureStrategyProfileRep> rep;

public:
  PureStrategyProfile(const PureStrategyProfile &r) : rep(r.rep->Copy()) {}

  explicit PureStrategyProfile(std::shared_ptr<PureStrategyProfileRep> p_rep) : rep(p_rep) {}

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
private:
  StrategySupportProfile m_support;
  std::vector<GamePlayer> m_unfrozen;
  std::vector<GameStrategy> m_frozen;

public:
  class iterator {
  private:
    StrategyContingencies *m_cont;
    bool m_atEnd;
    std::map<GamePlayer, StrategySupportProfile::Support::const_iterator> m_currentStrat;
    PureStrategyProfile m_profile;

  public:
    iterator(StrategyContingencies *, bool p_end);

    iterator &operator++();

    bool operator==(const iterator &p_other) const
    {
      if (m_atEnd && p_other.m_atEnd && m_cont == p_other.m_cont) {
        return true;
      }
      if (m_atEnd != p_other.m_atEnd || m_cont != p_other.m_cont) {
        return false;
      }
      return (m_profile.operator->() == p_other.m_profile.operator->());
    }
    bool operator!=(const iterator &p_other) const { return !(*this == p_other); }

    PureStrategyProfile &operator*() { return m_profile; }
    const PureStrategyProfile &operator*() const { return m_profile; }
  };

  explicit StrategyContingencies(const StrategySupportProfile &,
                                 const std::vector<GameStrategy> &p_frozen = {});
  iterator begin() { return {this, false}; }
  iterator end() { return {this, true}; }
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_STRATPURE_H
