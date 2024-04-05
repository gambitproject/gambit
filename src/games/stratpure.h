//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include "game.h"

namespace Gambit {

/// This class represents a strategy profile on a strategic game.
/// It specifies exactly one strategy for each player defined on the
/// game.
class PureStrategyProfileRep {
  friend class GameTableRep;

  friend class GameTreeRep;

  friend class GameAGGRep;

  friend class PureStrategyProfile;

protected:
  Game m_nfg;
  Array<GameStrategy> m_profile;

  /// Construct a new strategy profile
  explicit PureStrategyProfileRep(const Game &p_game);

  /// Create a copy of the strategy profile.
  /// Caller is responsible for memory management of the created object.
  virtual PureStrategyProfileRep *Copy() const = 0;

public:
  virtual ~PureStrategyProfileRep() = default;

  /// @name Data access and manipulation
  //@{
  /// Get the index uniquely identifying the strategy profile
  virtual long GetIndex() const { throw UndefinedException(); }

  /// Get the strategy played by player pl
  const GameStrategy &GetStrategy(int pl) const { return m_profile[pl]; }

  /// Get the strategy played by the player
  const GameStrategy &GetStrategy(const GamePlayer &p_player) const
  {
    return m_profile[p_player->GetNumber()];
  }

  /// Set the strategy for a player
  virtual void SetStrategy(const GameStrategy &) = 0;

  /// Get the outcome that results from the profile
  virtual GameOutcome GetOutcome() const = 0;

  /// Set the outcome that results from the profile
  virtual void SetOutcome(GameOutcome p_outcome) = 0;

  /// Get the payoff to player pl that results from the profile
  virtual Rational GetPayoff(int pl) const = 0;

  /// Get the payoff to the player resulting from the profile
  Rational GetPayoff(const GamePlayer &p_player) const { return GetPayoff(p_player->GetNumber()); }

  /// Get the value of playing strategy against the profile
  virtual Rational GetStrategyValue(const GameStrategy &) const = 0;

  /// Is the profile a pure strategy Nash equilibrium?
  bool IsNash() const;

  /// Is the profile a strict pure stategy Nash equilibrium?
  bool IsStrictNash() const;

  /// Is the specificed player playing a best response?
  bool IsBestResponse(const GamePlayer &p_player) const;

  /// Get the list of best response strategies for a player
  List<GameStrategy> GetBestResponse(const GamePlayer &p_player) const;

  /// Convert to a mixed strategy representation
  MixedStrategyProfile<Rational> ToMixedStrategyProfile() const;
  //@}
};

class PureStrategyProfile {
private:
  PureStrategyProfileRep *rep;

public:
  PureStrategyProfile(const PureStrategyProfile &r) : rep(r.rep->Copy()) {}

  explicit PureStrategyProfile(PureStrategyProfileRep *p_rep) : rep(p_rep) {}

  ~PureStrategyProfile() { delete rep; }

  PureStrategyProfile &operator=(const PureStrategyProfile &r)
  {
    if (&r != this) {
      delete rep;
      rep = r.rep->Copy();
    }
    return *this;
  }

  PureStrategyProfileRep *operator->() const { return rep; }

  explicit operator PureStrategyProfileRep *() const { return rep; }
};

/// This class iterates through the contingencies in a strategic game.
/// It visits each strategy profile in turn, advancing one contingency
/// on each call of NextContingency().  Optionally, the strategy of
/// one player may be held fixed during the iteration (by the use of the
/// second constructor).
class StrategyProfileIterator {
  friend class GameRep;
  friend class GameTableRep;

private:
  bool m_atEnd;
  StrategySupportProfile m_support;
  Array<int> m_currentStrat;
  PureStrategyProfile m_profile;
  int m_frozen1, m_frozen2;

  /// Reset the iterator to the first contingency (this is called by ctors)
  void First();

public:
  /// @name Lifecycle
  //@{
  /// Construct a new iterator on the support, with no strategies held fixed
  explicit StrategyProfileIterator(const StrategySupportProfile &);
  /// Construct a new iterator on the support, fixing player pl's strategy
  StrategyProfileIterator(const StrategySupportProfile &s, int pl, int st);
  /// Construct a new iterator on the support, fixing the given strategy
  StrategyProfileIterator(const StrategySupportProfile &, const GameStrategy &);
  /// Construct a new iterator on the support, fixing two players' strategies
  StrategyProfileIterator(const StrategySupportProfile &s, int pl1, int st1, int pl2, int st2);
  //@}

  /// @name Iteration and data access
  //@{
  /// Advance to the next contingency (prefix version)
  void operator++();
  /// Advance to the next contingency (postfix version)
  void operator++(int) { ++(*this); }
  /// Has iterator gone past the end?
  bool AtEnd() const { return m_atEnd; }

  /// Get the current strategy profile
  PureStrategyProfile &operator*() { return m_profile; }
  /// Get the current strategy profile
  const PureStrategyProfile &operator*() const { return m_profile; }
  //@}
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_STRATPURE_H
