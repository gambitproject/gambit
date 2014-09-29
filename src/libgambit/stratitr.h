//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/stratitr.h
// Normal form iterator class
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

#ifndef LIBGAMBIT_STRATITR_H
#define LIBGAMBIT_STRATITR_H

#include "game.h"

namespace Gambit {

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
  void First(void);

public:
  /// @name Lifecycle
  //@{
  /// Construct a new iterator on the support, with no strategies held fixed
  StrategyProfileIterator(const StrategySupportProfile &);
  /// Construct a new iterator on the support, fixing player pl's strategy
  StrategyProfileIterator(const StrategySupportProfile &s, int pl, int st);
  /// Construct a new iterator on the support, fixing the given strategy
  StrategyProfileIterator(const StrategySupportProfile &, const GameStrategy &);
  /// Construct a new iterator on the support, fixing two players' strategies
  StrategyProfileIterator(const StrategySupportProfile &s, 
			  int pl1, int st1, int pl2, int st2);
  //@}

  /// @name Iteration and data access
  //@{
  /// Advance to the next contingency (prefix version)
  void operator++(void);
  /// Advance to the next contingency (postfix version)
  void operator++(int) { ++(*this); }
  /// Has iterator gone past the end?
  bool AtEnd(void) const { return m_atEnd; }

  /// Get the current strategy profile
  PureStrategyProfile &operator*(void) { return m_profile; }
  /// Get the current strategy profile
  const PureStrategyProfile &operator*(void) const { return m_profile; }
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_STRATITR_H




