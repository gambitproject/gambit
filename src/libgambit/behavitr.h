//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/behavitr.h
// Interface to extensive form action iterators
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

#ifndef LIBGAMBIT_BEHAVITR_H
#define LIBGAMBIT_BEHAVITR_H

#include "behavspt.h"

namespace Gambit {

//
// N.B.: Currently, the contingency iterator only allows one information
//       set to be "frozen".  There is a list of "active" information
//       sets, which are those whose actions are cycled over, the idea
//       being that behavior at inactive information sets is irrelevant.
//
//       Iterating across all contingencies can be achieved by freezing
//       player number 0 (this is the default state on initialization)
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
  void First(void);

public:
  /// @name Lifecycle
  //@{
  /// Construct a new iterator on the support, with no actions held fixed
  BehaviorProfileIterator(const BehaviorSupportProfile &);
  /// Construct a new iterator on the support, holding the action fixed
  BehaviorProfileIterator(const BehaviorSupportProfile &, const GameAction &);
  //@}
  
  /// @name Iteration and data access
  //@{
  /// Advance to the next contingency (prefix version) 
  void operator++(void);
  /// Advance to the next contingency (postfix version) 
  void operator++(int) { ++(*this); }
  /// Has iterator gone past the end?
  bool AtEnd(void) const { return m_atEnd; }
  /// Get the current behavior profile
  const PureBehaviorProfile &operator*(void) const { return m_profile; }
  /// Get the current behavior profile
  const PureBehaviorProfile *const operator->(void) const { return &m_profile; }
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_BEHAVITR_H




