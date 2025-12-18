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

#include <vector>
#include "util.h"

namespace Gambit {

/// This class implements a rectangular (two-dimensional) array
template <class T> class RectArray {
protected:
  int m_minrow, m_maxrow, m_mincol, m_maxcol;
  std::vector<T> m_storage;

  size_t row_stride() const { return static_cast<size_t>(m_maxcol - m_mincol + 1); }

  size_t index(int r, int c) const
  {
    return static_cast<size_t>(r - m_minrow) * row_stride() + static_cast<size_t>(c - m_mincol);
  }

public:
  /// @name Lifecycle
  //@{
  RectArray() : m_minrow(1), m_maxrow(0), m_mincol(1), m_maxcol(0), m_storage() {}
  RectArray(const size_t nrows, const size_t ncols) : RectArray(1, nrows, 1, ncols) {}
  RectArray(const int minrow, const int maxrow, const int mincol, const int maxcol)
    : m_minrow(minrow), m_maxrow(maxrow), m_mincol(mincol), m_maxcol(maxcol),
      m_storage((maxrow >= minrow && maxcol >= mincol)
                    ? (maxrow - minrow + 1) * (maxcol - mincol + 1)
                    : 0)
  {
  }
  RectArray(const RectArray &) = default;
  RectArray(RectArray &&) noexcept = default;
  ~RectArray() = default;

  RectArray &operator=(const RectArray &) = default;
  RectArray &operator=(RectArray &&) noexcept = default;
  //@}

  /// @name Range checking functions; returns true only if valid index/size
  //@{
  /// check array for same row and column boundaries
  bool CheckBounds(const RectArray<T> &m) const
  {
    return (m_minrow == m.m_minrow && m_maxrow == m.m_maxrow && m_mincol == m.m_mincol &&
            m_maxcol == m.m_maxcol);
  }
  /// check for correct row index
  bool CheckRow(const int row) const { return (m_minrow <= row && row <= m_maxrow); }
  /// check row vector for correct column boundaries
  template <class V> bool CheckRow(const V &v) const
  {
    return v.front_index() == m_mincol && v.back_index() == m_maxcol;
  }
  /// check for correct column index
  bool CheckColumn(const int col) const { return (m_mincol <= col && col <= m_maxcol); }
  /// check column vector for correct row boundaries
  template <class V> bool CheckColumn(const V &v) const
  {
    return (v.front_index() == m_minrow && v.back_index() == m_maxrow);
  }
  /// check row and column indices
  bool Check(const int row, const int col) const { return CheckRow(row) && CheckColumn(col); }

  //@}
  /// @name General data access
  //@{
  size_t NumRows() const { return m_maxrow - m_minrow + 1; }
  size_t NumColumns() const { return m_maxcol - m_mincol + 1; }
  int MinRow() const { return m_minrow; }
  int MaxRow() const { return m_maxrow; }
  int MinCol() const { return m_mincol; }
  int MaxCol() const { return m_maxcol; }
  //@}

  /// @name Indexing operations
  //@{
  T &operator()(int r, int c)
  {
    if (!Check(r, c)) {
      throw std::out_of_range("Index out of range in RectArray");
    }
    return m_storage[index(r, c)];
  }
  const T &operator()(int r, int c) const
  {
    if (!Check(r, c)) {
      throw std::out_of_range("Index out of range in RectArray");
    }
    return m_storage[index(r, c)];
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

    const auto stride = row_stride();
    const size_t ai = index(i, m_mincol);
    const size_t aj = index(j, m_mincol);
    for (size_t k = 0; k < stride; ++k) {
      std::swap(m_storage[ai + k], m_storage[aj + k]);
    }
  }
  template <class Vector> void GetRow(int, Vector &) const;

  template <class Vector> void GetColumn(int, Vector &) const;
  template <class Vector> void SetColumn(int, const Vector &);
  //@}
};

//------------------------------------------------------------------------
//                   RectArray<T>: Row and column rotation
//------------------------------------------------------------------------

template <class T> void RectArray<T>::RotateUp(int lo, int hi)
{
  if (lo < m_minrow || hi < lo || m_maxrow < hi) {
    throw std::out_of_range("Index out of range in RectArray");
  }
  const auto stride = row_stride();
  const size_t first = index(lo, m_mincol);
  const size_t last = index(hi + 1, m_mincol);
  std::rotate(m_storage.begin() + first, m_storage.begin() + first + stride,
              m_storage.begin() + last);
}

template <class T> void RectArray<T>::RotateDown(int lo, int hi)
{
  if (lo < m_minrow || hi < lo || m_maxrow < hi) {
    throw std::out_of_range("Index out of range in RectArray");
  }
  const auto stride = row_stride();
  const size_t first = index(lo, m_mincol);
  const size_t last = index(hi + 1, m_mincol);
  std::rotate(m_storage.begin() + first, m_storage.begin() + last - stride,
              m_storage.begin() + last);
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
  const size_t base = index(row, m_mincol);
  for (int c = m_mincol; c <= m_maxcol; ++c) {
    v[c] = m_storage[base + (c - m_mincol)];
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
  for (int r = m_minrow; r <= m_maxrow; ++r) {
    v[r] = m_storage[index(r, col)];
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
  for (int r = m_minrow; r <= m_maxrow; ++r) {
    m_storage[index(r, col)] = v[r];
  }
}

} // end namespace Gambit

#endif // GAMBIT_CORE_RECARRAY_H
