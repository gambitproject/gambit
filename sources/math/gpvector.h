//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Partitioned vector class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef GPVECTOR_H
#define GPVECTOR_H

#include "math/gvector.h"

template <class T> class gPVector : public gVector<T> {
 private:
  int sum(const gArray<int> &V) const;
  void setindex(void);

 protected:
  T **svptr;
  gArray<int> svlen;

  int Check(const gPVector<T> &v) const;

 public:

  // constructors

  gPVector(void);
  gPVector(const gArray<int> &sig);
  gPVector(const gVector<T> &val, const gArray<int> &sig);
  gPVector(const gPVector<T> &v);
  virtual ~gPVector();


  // element access operators
  T& operator()(int a, int b);
  const T& operator()(int a, int b) const;

  // extract a subvector
  gVector<T> GetRow(int row) const;
  void GetRow(int row, gVector<T> &v) const;
  void SetRow(int row, const gVector<T> &v);
  void CopyRow(int row, const gPVector<T> &v);

  // more operators

  gPVector<T>& operator=(const gPVector<T> &v);
  gPVector<T>& operator=(const gVector<T> &v);
  gPVector<T>& operator=(T c);

  gPVector<T> operator+(const gPVector<T> &v) const;
  gPVector<T>& operator+=(const gPVector<T> &v);
  gPVector<T> operator-(void) const;
  gPVector<T> operator-(const gPVector<T> &v) const;
  gPVector<T>& operator-=(const gPVector<T> &v);
  T operator*(const gPVector<T> &v) const;
  gPVector<T> operator*(const T &c) const;
  gPVector<T>& operator*=(const T c);
  gPVector<T> operator/(T c);

  bool operator==(const gPVector<T> &v) const;
  bool operator!=(const gPVector<T> &v) const;

  // parameter access functions
  const gArray<int>& Lengths(void) const;

  void Dump(gOutput &) const;
};

#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &to, const gPVector<T> &v);
#endif

#endif   //# GPVECTOR_H



