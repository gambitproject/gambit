//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/indexproduct.h
// Utility class representing a product of index ranges
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

#ifndef INDEXPRODUCT_H
#define INDEXPRODUCT_H

#include "gambit.h"

namespace Gambit {

// A representation of a cartesian product of indices, with an associated iterator
class CartesianIndexProduct {
private:
  Array<int> m_lower, m_upper;

public:
  class iterator {
  private:
    CartesianIndexProduct *m_set;
    Array<int> m_current;
    bool m_end;

  public:
    iterator(CartesianIndexProduct *p_set, bool p_end)
      : m_set(p_set), m_current(p_set->m_lower), m_end(p_end)
    {
    }
    iterator(const iterator &) = default;
    ~iterator() = default;

    const Array<int> &operator*() const { return m_current; }

    bool operator==(const iterator &p_other) const
    {
      if (p_other.m_set != m_set || p_other.m_end != m_end) {
        return false;
      }
      return m_end || m_current == p_other.m_current;
    }
    bool operator!=(const iterator &p_other) const { return !(*this == p_other); }
    iterator &operator++()
    {
      auto [cur, up] = std::mismatch(m_current.begin(), m_current.end(), m_set->m_upper.begin());
      if (cur == m_current.end()) {
        m_end = true;
      }
      else {
        std::copy(m_current.begin(), cur, m_set->m_lower.begin());
        (*cur)++;
      }
      return *this;
    }
  };

  CartesianIndexProduct(const Array<int> &p_lower, const Array<int> &p_upper)
    : m_lower(p_lower), m_upper(p_upper)
  {
  }
  CartesianIndexProduct(const CartesianIndexProduct &) = default;
  ~CartesianIndexProduct() = default;

  CartesianIndexProduct &operator=(const CartesianIndexProduct &) = default;

  iterator begin() { return iterator(this, false); }
  iterator end() { return iterator(this, true); }
};

} // namespace Gambit

#endif // INDEXPRODUCT_H
