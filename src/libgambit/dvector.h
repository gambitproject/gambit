//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/dvector.h
// Doubly-partitioned vector class
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

#ifndef LIBGAMBIT_DVECTOR_H
#define LIBGAMBIT_DVECTOR_H

#include "pvector.h"

namespace Gambit {

template <class T> class DVector : public PVector<T>  {
private:
  int sum(int part, const PVector<int> &v) const;
  void setindex(void);

  bool Check(const DVector<T> &) const;

protected:
  T ***dvptr;
  Array<int> dvlen, dvidx;

public:
  DVector(void);
  DVector(const PVector<int> &sig);
  DVector(const Vector<T> &val, const PVector<int> &sig);
  DVector(const DVector<T> &v);
  virtual ~DVector();

  T &operator()(int a, int b, int c);
  const T &operator()(int a, int b, int c) const;

  // extract a subvector
  void CopySubRow(int row, int col,  const DVector<T> &v);

  DVector<T> &operator=(const DVector<T> &v);
  DVector<T> &operator=(const PVector<T> &v);
  DVector<T> &operator=(const Vector<T> &v);
  DVector<T> &operator=(T c);

  DVector<T> operator+(const DVector<T> &v) const;
  DVector<T> &operator+=(const DVector<T> &v);
  DVector<T> operator-(void) const;
  DVector<T> operator-(const DVector<T> &v) const;
  DVector<T> &operator-=(const DVector<T> &v);
  T operator*(const DVector<T> &v) const;
  DVector<T> &operator*=(const T &c);
  DVector<T> operator/(const T &c) const;
    
  bool operator==(const DVector<T> &v) const;
  bool operator!=(const DVector<T> &v) const;

  const Array<int> &DPLengths(void) const;
};

} // end namespace Gambit

#endif // LIBGAMBIT_DVECTOR_H



