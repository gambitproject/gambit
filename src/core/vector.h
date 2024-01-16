//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

namespace Gambit {

template <class T> class Matrix;

/// A mathematical vector: a list of numbers with the standard math operators
template <class T> class Vector : public Array<T> {
  friend class Matrix<T>;

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
  Vector<T> &operator=(const Vector<T> &V);
  /** Assigns the value c to all components of the vector */
  Vector<T> &operator=(T c);

  Vector<T> operator+(const Vector<T> &V) const;
  Vector<T> &operator+=(const Vector<T> &V);

  Vector<T> operator-();
  Vector<T> operator-(const Vector<T> &V) const;
  Vector<T> &operator-=(const Vector<T> &V);

  Vector<T> operator*(T c) const;
  Vector<T> &operator*=(T c);
  T operator*(const Vector<T> &V) const;

  Vector<T> operator/(T c) const;

  bool operator==(const Vector<T> &V) const;
  bool operator!=(const Vector<T> &V) const { return !(*this == V); }

  /** Tests if all components of the vector are equal to a constant c */
  bool operator==(T c) const;
  bool operator!=(T c) const;

  // square of length
  T NormSquared() const;

  // check vector for identical boundaries
  bool Check(const Vector<T> &v) const;
};

} // end namespace Gambit

#endif // LIBGAMBIT_VECTOR_H
