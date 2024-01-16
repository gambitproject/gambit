//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/pvector.h
// Partitioned vector class
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

#ifndef LIBGAMBIT_PVECTOR_H
#define LIBGAMBIT_PVECTOR_H

#include "vector.h"

namespace Gambit {

template <class T> class PVector : public Vector<T> {
private:
  int sum(const Array<int> &V) const;
  void setindex();

protected:
  T **svptr;
  Array<int> svlen;

  int Check(const PVector<T> &v) const;

public:
  // constructors

  PVector();
  explicit PVector(const Array<int> &sig);
  PVector(const Vector<T> &val, const Array<int> &sig);
  PVector(const PVector<T> &v);
  ~PVector() override;

  // element access operators
  T &operator()(int a, int b);
  const T &operator()(int a, int b) const;

  // extract a subvector
  Vector<T> GetRow(int row) const;
  void GetRow(int row, Vector<T> &v) const;
  void SetRow(int row, const Vector<T> &v);
  void CopyRow(int row, const PVector<T> &v);

  // more operators

  PVector<T> &operator=(const PVector<T> &v);
  PVector<T> &operator=(const Vector<T> &v);
  PVector<T> &operator=(T c);

  PVector<T> operator+(const PVector<T> &v) const;
  PVector<T> &operator+=(const PVector<T> &v);
  PVector<T> operator-() const;
  PVector<T> operator-(const PVector<T> &v) const;
  PVector<T> &operator-=(const PVector<T> &v);
  T operator*(const PVector<T> &v) const;
  PVector<T> operator*(const T &c) const;
  PVector<T> &operator*=(const T &c);
  PVector<T> operator/(T c);

  bool operator==(const PVector<T> &v) const;
  bool operator!=(const PVector<T> &v) const;

  // parameter access functions
  const Array<int> &Lengths() const;
};

} // end namespace Gambit

#endif // LIBGAMBIT_PVECTOR_H
