//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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
  Array<Array<GameAction> > m_profile;

public:
  /// @name Lifecycle
  //@{
  /// Construct a new behavior profile on the specified game
  explicit PureBehaviorProfile(Game);

  /// @name Data access and manipulation
  //@{
  /// Get the action played at an information set
  GameAction GetAction(const GameInfoset &) const;

  /// Set the action played at an information set
  void SetAction(const GameAction &);

  /// Get the payoff to player pl that results from the profile
  template <class T> T GetPayoff(int pl) const;
  /// Get the payoff to the player that results from the profile
  template <class T> T GetPayoff(const GamePlayer &p_player) const
  { return GetPayoff<T>(p_player->GetNumber()); }
  /// Get the payoff to player pl conditional on reaching a node
  template <class T> T GetPayoff(const GameNode &, int pl) const;
  /// Get the payoff to playing the action, conditional on the profile
  template <class T> T GetPayoff(const GameAction &) const;

  /// Is the profile a pure strategy agent Nash equilibrium?
  bool IsAgentNash() const;

  /// Convert to a mixed behavior representation
  MixedBehaviorProfile<Rational> ToMixedBehaviorProfile() const;
  //@}
};

template<> inline double PureBehaviorProfile::GetPayoff(int pl) const
{ return GetPayoff<double>(m_efg->GetRoot(), pl); }

template<> inline Rational PureBehaviorProfile::GetPayoff(int pl) const
{ return GetPayoff<Rational>(m_efg->GetRoot(), pl); }

template<> inline std::string PureBehaviorProfile::GetPayoff(int pl) const
{ return lexical_cast<std::string>(GetPayoff<Rational>(m_efg->GetRoot(), pl)); }


//
// Currently, the contingency iterator only allows one information
// set to be "frozen".  There is a list of "active" information
// sets, which are those whose actions are cycled over, the idea
// being that behavior at inactive information sets is irrelevant.
//
class BehaviorProfileIterator {
private:
  bool m_atEnd;
  BehaviorSupportProfile m_support;
  PVector<int> m_currentBehav;
  PureBehaviorProfile m_profile;
  int m_frozenPlayer, m_frozenInfoset;
  Array<Array<bool> > m_isActive;
  Array<int> m_numActiveInfosets;

  /// Reset the iterator to the first contingency (this is called by ctors)
  void First();

public:
  /// @name Lifecycle
  //@{
  /// Construct a new iterator on the game, with no actions held fixed
  explicit BehaviorProfileIterator(const Game &);
  /// Construct a new iterator on the support, holding the action fixed
  BehaviorProfileIterator(const BehaviorSupportProfile &, const GameAction &);
  //@}
  
  /// @name Iteration and data access
  //@{
  /// Advance to the next contingency (prefix version) 
  void operator++();
  /// Advance to the next contingency (postfix version) 
  void operator++(int) { ++(*this); }
  /// Has iterator gone past the end?
  bool AtEnd() const { return m_atEnd; }
  /// Get the current behavior profile
  const PureBehaviorProfile &operator*() const { return m_profile; }
  //@}
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_BEHAVPURE_H




