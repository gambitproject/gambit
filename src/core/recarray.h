//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/recarray.h
// Rectangular array base class
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

#ifndef GAMBIT_CORE_RECARRAY_H
#define GAMBIT_CORE_RECARRAY_H

#include "util.h"

namespace Gambit {

/// This class implements a rectangular (two-dimensional) array
template <class T> class RectArray {
protected:
  int minrow, maxrow, mincol, maxcol;
  T **data;

  /// @name Range checking functions; returns true only if valid index/size
  //@{
  /// check for correct row index
  bool CheckRow(int row) const { return (minrow <= row && row <= maxrow); }
  /// check row vector for correct column boundaries
  template <class Vector> bool CheckRow(const Vector &v) const
  {
    return (v.front_index() == mincol && v.back_index() == maxcol);
  }
  /// check for correct column index
  bool CheckColumn(int col) const { return (mincol <= col && col <= maxcol); }
  /// check column vector for correct row boundaries
  template <class Vector> bool CheckColumn(const Vector &v) const
  {
    return (v.front_index() == minrow && v.back_index() == maxrow);
  }
  /// check row and column indices
  bool Check(int row, int col) const { return CheckRow(row) && CheckColumn(col); }
  /// check matrix for same row and column boundaries
  bool CheckBounds(const RectArray<T> &m) const
  {
    return (minrow == m.minrow && maxrow == m.maxrow && mincol == m.mincol && maxcol == m.maxcol);
  }
  //@}

public:
  /// @name Lifecycle
  //@{
  RectArray() : minrow(1), maxrow(0), mincol(1), maxcol(0), data(nullptr) {}
  RectArray(unsigned int nrows, unsigned int ncols);
  RectArray(int minr, int maxr, int minc, int maxc);
  RectArray(const RectArray<T> &);
  virtual ~RectArray();

  RectArray<T> &operator=(const RectArray<T> &);
  //@}

  /// @name General data access
  //@{
  size_t NumRows() const { return maxrow - minrow + 1; }
  size_t NumColumns() const { return maxcol - mincol + 1; }
  int MinRow() const { return minrow; }
  int MaxRow() const { return maxrow; }
  int MinCol() const { return mincol; }
  int MaxCol() const { return maxcol; }
  //@}

  /// @name Indexing operations
  //@{
  T &operator()(int r, int c)
  {
    if (!Check(r, c)) {
      throw std::out_of_range("Index out of range in RectArray");
    }
    return data[r][c];
  }
  const T &operator()(int r, int c) const
  {
    if (!Check(r, c)) {
      throw std::out_of_range("Index out of range in RectArray");
    }
    return data[r][c];
  }
  //@}

  /// @name Row and column rotation operators
  //@{
  void RotateUp(int lo, int hi);
  void RotateDown(int lo, int hi);
  //@}

  /// @name Row and column manipulation functions
  //@{
  void SwitchRows(int i, int j)
  {
    if (!Check(i, j)) {
      throw std::out_of_range("Index out of range in RectArray");
    }
    std::swap(data[i], data[j]);
  }
  template <class Vector> void GetRow(int, Vector &) const;

  template <class Vector> void GetColumn(int, Vector &) const;
  template <class Vector> void SetColumn(int, const Vector &);
  //@}
};

//------------------------------------------------------------------------
//     RectArray<T>: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

template <class T>
RectArray<T>::RectArray(unsigned int rows, unsigned int cols)
  : minrow(1), maxrow(rows), mincol(1), maxcol(cols),
    data((rows > 0) ? new T *[maxrow] - 1 : nullptr)
{
  for (int i = 1; i <= maxrow; data[i++] = (cols > 0) ? new T[maxcol] - 1 : nullptr)
    ;
}

template <class T>
RectArray<T>::RectArray(int minr, int maxr, int minc, int maxc)
  : minrow(minr), maxrow(maxr), mincol(minc), maxcol(maxc),
    data((maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : nullptr)
{
  for (int i = minrow; i <= maxrow;
       data[i++] = (maxcol - mincol + 1) ? new T[maxcol - mincol + 1] - mincol : nullptr)
    ;
}

template <class T>
RectArray<T>::RectArray(const RectArray<T> &a)
  : minrow(a.minrow), maxrow(a.maxrow), mincol(a.mincol), maxcol(a.maxcol),
    data((maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : nullptr)
{
  for (int i = minrow; i <= maxrow; i++) {
    data[i] = (maxcol >= mincol) ? new T[maxcol - mincol + 1] - mincol : nullptr;
    for (int j = mincol; j <= maxcol; j++) {
      data[i][j] = a.data[i][j];
    }
  }
}

template <class T> RectArray<T>::~RectArray()
{
  for (int i = minrow; i <= maxrow; i++) {
    if (data[i]) {
      delete[] (data[i] + mincol);
    }
  }
  if (data) {
    delete[] (data + minrow);
  }
}

template <class T> RectArray<T> &RectArray<T>::operator=(const RectArray<T> &a)
{
  if (this != &a) {
    for (int i = minrow; i <= maxrow; i++) {
      if (data[i]) {
        delete[] (data[i] + mincol);
      }
    }
    if (data) {
      delete[] (data + minrow);
    }

    minrow = a.minrow;
    maxrow = a.maxrow;
    mincol = a.mincol;
    maxcol = a.maxcol;

    data = (maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : nullptr;

    for (int i = minrow; i <= maxrow; i++) {
      data[i] = (maxcol >= mincol) ? new T[maxcol - mincol + 1] - mincol : nullptr;
      for (int j = mincol; j <= maxcol; j++) {
        data[i][j] = a.data[i][j];
      }
    }
  }

  return *this;
}

//------------------------------------------------------------------------
//                   RectArray<T>: Row and column rotation
//------------------------------------------------------------------------

template <class T> void RectArray<T>::RotateUp(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi) {
    throw std::out_of_range("Index out of range in RectArray");
  }

  T *temp = data[lo];
  for (int k = lo; k < hi; k++) {
    data[k] = data[k + 1];
  }
  data[hi] = temp;
}

template <class T> void RectArray<T>::RotateDown(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi) {
    throw std::out_of_range("Index out of range in RectArray");
  }

  T *temp = data[hi];
  for (int k = hi; k > lo; k--) {
    data[k] = data[k - 1];
  }
  data[lo] = temp;
}

//-------------------------------------------------------------------------
//                 RectArray<T>: Row manipulation functions
//-------------------------------------------------------------------------

template <class T> template <class Vector> void RectArray<T>::GetRow(int row, Vector &v) const
{
  if (!CheckRow(row)) {
    throw std::out_of_range("Index out of range in RectArray");
  }
  if (!CheckRow(v)) {
    throw DimensionException();
  }
  const T *rowptr = data[row];
  for (int i = mincol; i <= maxcol; i++) {
    v[i] = rowptr[i];
  }
}

//-------------------------------------------------------------------------
//                RectArray<T>: Column manipulation functions
//-------------------------------------------------------------------------

template <class T> template <class Vector> void RectArray<T>::GetColumn(int col, Vector &v) const
{
  if (!CheckColumn(col)) {
    throw std::out_of_range("Index out of range in RectArray");
  }
  if (!CheckColumn(v)) {
    throw DimensionException();
  }
  for (int i = minrow; i <= maxrow; i++) {
    v[i] = data[i][col];
  }
}

template <class T> template <class Vector> void RectArray<T>::SetColumn(int col, const Vector &v)
{
  if (!CheckColumn(col)) {
    throw std::out_of_range("Index out of range in RectArray");
  }
  if (!CheckColumn(v)) {
    throw DimensionException();
  }
  for (int i = minrow; i <= maxrow; i++) {
    data[i][col] = v[i];
  }
}

} // end namespace Gambit

#endif // GAMBIT_CORE_RECARRAY_H
