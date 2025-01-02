//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/odometer.h
// Declaration of gIndexOdometer class
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

/*
   When the cartesian product of ordered sets is ordered
lexicographically, there is a relation between such a relation,
and the numerical indexing derived from the lexicographical
ordering, that is similar to an odometer.  Here the least
significant index is the first ("leftmost").
*/

#ifndef ODOMETER_H
#define ODOMETER_H

#include "gambit.h"

namespace Gambit {

// An iterator over the cartesian product of sets
class CartesianIndexIterator {
private:
  Array<int> m_lower, m_upper, m_current;

public:
  CartesianIndexIterator(const Array<int> &p_lower, const Array<int> &p_upper)
    : m_lower(p_lower), m_upper(p_upper), m_current(p_lower)
  {
    m_current.front() = m_lower.front() - 1;
  }

  CartesianIndexIterator(const CartesianIndexIterator &) = default;

  ~CartesianIndexIterator() = default;

  // Operators
  CartesianIndexIterator &operator=(const CartesianIndexIterator &) = default;

  bool operator==(const CartesianIndexIterator &p_rhs) const
  {
    return m_lower == p_rhs.m_lower && m_upper == p_rhs.m_upper && m_current == p_rhs.m_current;
  }

  bool operator!=(const CartesianIndexIterator &p_rhs) const
  {
    return m_lower != p_rhs.m_lower || m_upper != p_rhs.m_upper || m_current != p_rhs.m_current;
  }

  int operator[](int place) const { return m_current[place]; }

  bool Turn()
  {
    auto [cur, up] = std::mismatch(m_current.begin(), m_current.end(), m_upper.begin());
    if (cur == m_current.end()) {
      return false;
    }
    std::copy(m_current.begin(), cur, m_lower.begin());
    (*cur)++;
    return true;
  }
};

} // namespace Gambit

#endif // ODOMETER_H
