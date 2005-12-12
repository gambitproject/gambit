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

#include "gvector.h"

template <class T> class gbtPVector : public gbtVector<T> {
 private:
  int sum(const gbtArray<int> &V) const;
  void setindex(void);

 protected:
  T **svptr;
  gbtArray<int> svlen;

  int Check(const gbtPVector<T> &v) const;

 public:

  // constructors

  gbtPVector(void);
  gbtPVector(const gbtArray<int> &sig);
  gbtPVector(const gbtVector<T> &val, const gbtArray<int> &sig);
  gbtPVector(const gbtPVector<T> &v);
  virtual ~gbtPVector();


  // element access operators
  T& operator()(int a, int b);
  const T& operator()(int a, int b) const;

  // extract a subvector
  gbtVector<T> GetRow(int row) const;
  void GetRow(int row, gbtVector<T> &v) const;
  void SetRow(int row, const gbtVector<T> &v);
  void CopyRow(int row, const gbtPVector<T> &v);

  // more operators

  gbtPVector<T>& operator=(const gbtPVector<T> &v);
  gbtPVector<T>& operator=(const gbtVector<T> &v);
  gbtPVector<T>& operator=(T c);

  gbtPVector<T> operator+(const gbtPVector<T> &v) const;
  gbtPVector<T>& operator+=(const gbtPVector<T> &v);
  gbtPVector<T> operator-(void) const;
  gbtPVector<T> operator-(const gbtPVector<T> &v) const;
  gbtPVector<T>& operator-=(const gbtPVector<T> &v);
  T operator*(const gbtPVector<T> &v) const;
  gbtPVector<T> operator*(const T &c) const;
  gbtPVector<T>& operator*=(const T c);
  gbtPVector<T> operator/(T c);

  bool operator==(const gbtPVector<T> &v) const;
  bool operator!=(const gbtPVector<T> &v) const;

  // parameter access functions
  const gbtArray<int>& Lengths(void) const;
};

#endif   //# GPVECTOR_H



