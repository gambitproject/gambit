//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/vector.h
// A vector class
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

#ifndef GAMBIT_CORE_VECTOR_H
#define GAMBIT_CORE_VECTOR_H

#include <numeric>

#include "array.h"

namespace Gambit {

template <class T> class Matrix;

/// A mathematical vector: a list of numbers with the standard math operators
template <class T> class Vector {
  Array<T> m_data;

  bool is_conformable(const Vector &p_other) const
  {
    return p_other.front_index() == front_index() && p_other.size() == size();
  }

public:
  using iterator = typename Array<T>::iterator;
  using const_iterator = typename Array<T>::const_iterator;

  explicit Vector(size_t len = 0) : m_data(len) {}
  Vector(int low, int high) : m_data(low, high) {}
  Vector(Vector &&) noexcept = default;
  Vector(const Vector &) = default;
  ~Vector() = default;

  Vector &operator=(Vector &&v) noexcept = default;
  Vector &operator=(const Vector &v)
  {
    if (!is_conformable(v)) {
      throw DimensionException();
    }
    m_data = v.m_data;
    return *this;
  }
  Vector &operator=(const T &c)
  {
    std::fill(m_data.begin(), m_data.end(), c);
    return *this;
  }

  int front_index() const noexcept { return m_data.front_index(); }
  int back_index() const noexcept { return m_data.back_index(); }
  size_t size() const noexcept { return m_data.size(); }

  const T &front() const { return m_data.front(); }
  T &front() { return m_data.front(); }
  const T &back() const { return m_data.back(); }
  T &back() { return m_data.back(); }

  const T &operator[](int i) const { return m_data[i]; }
  T &operator[](int i) { return m_data[i]; }

  iterator begin() { return m_data.begin(); }
  const_iterator begin() const { return m_data.begin(); }
  iterator end() { return m_data.end(); }
  const_iterator end() const { return m_data.end(); }
  const_iterator cbegin() const { return m_data.cbegin(); }
  const_iterator cend() const { return m_data.cend(); }

  Vector operator+(const Vector &v) const
  {
    if (!is_conformable(v)) {
      throw DimensionException();
    }
    Vector tmp(front_index(), back_index());
    std::transform(m_data.cbegin(), m_data.cend(), v.m_data.cbegin(), tmp.m_data.begin(),
                   std::plus<>());
    return tmp;
  }

  Vector &operator+=(const Vector &v)
  {
    if (!is_conformable(v)) {
      throw DimensionException();
    }
    std::transform(m_data.cbegin(), m_data.cend(), v.m_data.cbegin(), m_data.begin(),
                   std::plus<>());
    return *this;
  }

  Vector operator-(const Vector &v) const
  {
    if (!is_conformable(v)) {
      throw DimensionException();
    }
    Vector tmp(front_index(), back_index());
    std::transform(m_data.cbegin(), m_data.cend(), v.m_data.cbegin(), tmp.m_data.begin(),
                   std::minus<>());
    return tmp;
  }

  Vector &operator-=(const Vector &v)
  {
    if (!is_conformable(v)) {
      throw DimensionException();
    }
    std::transform(m_data.cbegin(), m_data.cend(), v.m_data.cbegin(), m_data.begin(),
                   std::minus<>());
    return *this;
  }

  Vector operator*(const T &c) const
  {
    Vector tmp(front_index(), back_index());
    std::transform(m_data.cbegin(), m_data.cend(), tmp.m_data.begin(),
                   [&c](const T &v) { return v * c; });
    return tmp;
  }

  Vector &operator*=(const T &c)
  {
    std::transform(m_data.begin(), m_data.end(), m_data.begin(),
                   [&c](const T &v) { return v * c; });
    return *this;
  }

  T operator*(const Vector &v) const
  {
    if (!is_conformable(v)) {
      throw DimensionException();
    }
    return std::inner_product(m_data.cbegin(), m_data.cend(), v.m_data.cbegin(),
                              static_cast<T>(0));
  }

  Vector operator/(const T &c) const
  {
    Vector tmp(front_index(), back_index());
    std::transform(m_data.cbegin(), m_data.cend(), tmp.m_data.begin(),
                   [&c](const T &v) { return v / c; });
    return tmp;
  }

  Vector &operator/=(const T &c)
  {
    std::transform(m_data.cbegin(), m_data.cend(), m_data.begin(),
                   [&c](const T &v) { return v / c; });
    return *this;
  }

  bool operator==(const Vector &v) const { return m_data == v.m_data; }
  bool operator!=(const Vector &v) const { return m_data != v.m_data; }

  /// Tests if all components of the vector are equal to a constant c
  bool operator==(const T &c) const
  {
    return std::all_of(m_data.begin(), m_data.end(), [&c](const T &v) { return v == c; });
  }
  bool operator!=(const T &c) const
  {
    return std::any_of(m_data.begin(), m_data.end(), [&c](const T &v) { return v != c; });
  }

  /// The square of the Euclidean norm of the vector
  T NormSquared() const noexcept(noexcept(std::declval<T>() * std::declval<T>()))
  {
    return std::transform_reduce(m_data.cbegin(), m_data.cend(), T{0}, std::plus<>{},
                                 [](const T &v) -> T { return v * v; });
  }
};

template <class T> Vector<T> operator*(const T &p_c, const Vector<T> &p_vector)
{
  return p_vector * p_c;
}

} // end namespace Gambit

#endif // GAMBIT_CORE_VECTOR_H
