//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/vector.h
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

#ifndef LIBGAMBIT_VECTOR_H
#define LIBGAMBIT_VECTOR_H

#include <numeric>

namespace Gambit {

template <class T> class Matrix;

/// A mathematical vector: a list of numbers with the standard math operators
template <class T> class Vector : public Array<T> {
private:
  // check vector for identical boundaries
  bool Check(const Vector<T> &v) const
  {
    return (v.first_index() == this->first_index() && v.size() == this->size());
  }

public:
  /** Create a vector of length len, starting at 1 */
  explicit Vector(unsigned int len = 0) : Array<T>(len) {}
  /** Create a vector indexed from low to high */
  Vector(int low, int high) : Array<T>(low, high) {}
  /** Copy constructor */
  Vector(const Vector<T> &) = default;
  /** Destructor */
  ~Vector() override = default;

  /** Assignment operator: requires vectors to be of same length */
  Vector<T> &operator=(const Vector<T> &V)
  {
    if (!Check(V)) {
      throw DimensionException();
    }
    Array<T>::operator=(V);
    return *this;
  }
  /** Assigns the value c to all components of the vector */
  Vector<T> &operator=(const T &c)
  {
    std::fill(this->begin(), this->end(), c);
    return *this;
  }

  Vector<T> operator+(const Vector<T> &V) const
  {
    if (!Check(V)) {
      throw DimensionException();
    }
    Vector<T> tmp(this->first_index(), this->last_index());
    std::transform(this->cbegin(), this->cend(), V.cbegin(), tmp.begin(), std::plus<>());
    return tmp;
  }

  Vector<T> &operator+=(const Vector<T> &V)
  {
    if (!Check(V)) {
      throw DimensionException();
    }
    std::transform(this->cbegin(), this->cend(), V.cbegin(), this->begin(), std::plus<>());
    return *this;
  }

  Vector<T> operator-(const Vector<T> &V) const
  {
    if (!Check(V)) {
      throw DimensionException();
    }
    Vector<T> tmp(this->first_index(), this->last_index());
    std::transform(this->cbegin(), this->cend(), V.cbegin(), tmp.begin(), std::minus<>());
    return tmp;
  }

  Vector<T> &operator-=(const Vector<T> &V)
  {
    if (!Check(V)) {
      throw DimensionException();
    }
    std::transform(this->cbegin(), this->cend(), V.cbegin(), this->begin(), std::minus<>());
    return *this;
  }

  Vector<T> operator*(const T &c) const
  {
    Vector<T> tmp(this->first_index(), this->last_index());
    std::transform(this->cbegin(), this->cend(), tmp.begin(), [&](const T &v) { return v * c; });
    return tmp;
  }

  Vector<T> &operator*=(const T &c)
  {
    std::transform(this->cbegin(), this->cend(), this->begin(), [&](const T &v) { return v * c; });
    return *this;
  }

  T operator*(const Vector<T> &V) const
  {
    if (!Check(V)) {
      throw DimensionException();
    }
    return std::inner_product(this->begin(), this->end(), V.begin(), static_cast<T>(0));
  }

  Vector<T> operator/(const T &c) const
  {
    Vector<T> tmp(this->first_index(), this->last_index());
    std::transform(this->cbegin(), this->cend(), tmp.begin(), [&](const T &v) { return v / c; });
    return tmp;
  }

  Vector<T> &operator/=(const T &c)
  {
    std::transform(this->cbegin(), this->cend(), this->begin(), [&](const T &v) { return v / c; });
    return *this;
  }

  bool operator==(const Vector<T> &V) const
  {
    if (!Check(V)) {
      throw DimensionException();
    }
    return Array<T>::operator==(V);
  }
  bool operator!=(const Vector<T> &V) const { return !(*this == V); }

  /** Tests if all components of the vector are equal to a constant c */
  bool operator==(const T &c) const
  {
    return std::all_of(this->begin(), this->end(), [&](const T &v) { return v == c; });
  }
  bool operator!=(const T &c) const
  {
    return std::any_of(this->begin(), this->end(), [&](const T &v) { return v != c; });
  }

  // square of length
  T NormSquared() const
  {
    return std::inner_product(this->begin(), this->end(), this->begin(), static_cast<T>(0));
  }
};

} // end namespace Gambit

#endif // LIBGAMBIT_VECTOR_H
