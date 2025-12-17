//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
  std::vector<T> storage;

  size_t row_stride() const { return static_cast<size_t>(maxcol - mincol + 1); }

  size_t index(int r, int c) const
  {
    return static_cast<size_t>(r - minrow) * row_stride() + static_cast<size_t>(c - mincol);
  }

public:
  /// @name Lifecycle
  //@{
  RectArray() : minrow(1), maxrow(0), mincol(1), maxcol(0), storage() {}
  RectArray(unsigned int nrows, unsigned int ncols);
  RectArray(int minr, int maxr, int minc, int maxc)
    : minrow(minr), maxrow(maxr), mincol(minc), maxcol(maxc),
      storage((maxr >= minr && maxc >= minc) ? (maxr - minr + 1) * (maxc - minc + 1) : 0)
  {
  }
  RectArray(const RectArray &) = default;
  RectArray(RectArray &&) noexcept = default;

  virtual ~RectArray() = default;

  RectArray &operator=(const RectArray &) = default;
  RectArray &operator=(RectArray &&) noexcept = default;
  //@}

  /// check array for same row and column boundaries
  bool CheckBounds(const RectArray<T> &m) const
  {
    return (minrow == m.minrow && maxrow == m.maxrow && mincol == m.mincol && maxcol == m.maxcol);
  }
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
    return storage[index(r, c)];
  }
  const T &operator()(int r, int c) const
  {
    if (!Check(r, c)) {
      throw std::out_of_range("Index out of range in RectArray");
    }
    return storage[index(r, c)];
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
    if (i == j) {
      return;
    }

    auto stride = row_stride();
    size_t ai = index(i, mincol);
    size_t aj = index(j, mincol);

    for (size_t k = 0; k < stride; ++k) {
      std::swap(storage[ai + k], storage[aj + k]);
    }
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
RectArray<T>::RectArray(unsigned int rows, unsigned int cols) : RectArray(1, rows, 1, cols)
{
}

//------------------------------------------------------------------------
//                   RectArray<T>: Row and column rotation
//------------------------------------------------------------------------

template <class T> void RectArray<T>::RotateUp(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi) {
    throw std::out_of_range("Index out of range in RectArray");
  }

  auto stride = row_stride();

  size_t first = index(lo, mincol);
  size_t last = index(hi + 1, mincol);

  std::rotate(storage.begin() + first, storage.begin() + first + stride, storage.begin() + last);
}

template <class T> void RectArray<T>::RotateDown(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi) {
    throw std::out_of_range("Index out of range in RectArray");
  }

  auto stride = row_stride();

  size_t first = index(lo, mincol);
  size_t last = index(hi + 1, mincol);

  std::rotate(storage.begin() + first, storage.begin() + last - stride, storage.begin() + last);
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
  size_t base = index(row, mincol);
  for (int c = mincol; c <= maxcol; ++c) {
    v[c] = storage[base + (c - mincol)];
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
  for (int r = minrow; r <= maxrow; ++r) {
    v[r] = storage[index(r, col)];
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
  for (int r = minrow; r <= maxrow; ++r) {
    storage[index(r, col)] = v[r];
  }
}

} // end namespace Gambit

#endif // GAMBIT_CORE_RECARRAY_H
