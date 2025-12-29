//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/matrix.h
// Interface to a matrix class
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

#ifndef GAMBIT_CORE_MATRIX_H
#define GAMBIT_CORE_MATRIX_H

#include "recarray.h"
#include "vector.h"
#include "rational.h"

namespace Gambit {

class SingularMatrixException final : public std::runtime_error {
public:
  SingularMatrixException() : std::runtime_error("Attempted to invert a singular matrix") {}
  ~SingularMatrixException() noexcept override = default;
};

template <class T> class Matrix {
  RectArray<T> m_data;

public:
  /// @name Lifecycle
  //@{
  Matrix() = default;
  Matrix(unsigned int rows, unsigned int cols) : m_data(rows, cols) {}
  Matrix(unsigned int rows, unsigned int cols, int minrows)
    : m_data(minrows, minrows + rows - 1, 1, cols)
  {
  }
  Matrix(int rl, int rh, int cl, int ch) : m_data(rl, rh, cl, ch) {}
  Matrix(const Matrix &) = default;
  Matrix(Matrix &&) noexcept = default;
  ~Matrix() = default;

  Matrix &operator=(const Matrix &) = default;
  Matrix &operator=(Matrix &&) noexcept = default;
  Matrix &operator=(const T &);
  //@}

  // element access
  T &operator()(int r, int c) { return m_data(r, c); }
  const T &operator()(int r, int c) const { return m_data(r, c); }

  // bounds / dimensions
  int MinRow() const { return m_data.MinRow(); }
  int MaxRow() const { return m_data.MaxRow(); }
  int MinCol() const { return m_data.MinCol(); }
  int MaxCol() const { return m_data.MaxCol(); }
  int NumRows() const { return m_data.NumRows(); }
  int NumColumns() const { return m_data.NumColumns(); }

  bool IsSquare() const { return MinRow() == MinCol() && MaxRow() == MaxCol(); }

  // row ops used internally
  void SwitchRows(int i, int j) { m_data.SwitchRows(i, j); }
  template <class V> void GetColumn(int j, V &) const;
  template <class V> void SetColumn(int j, const V &);
  template <class V> void GetRow(int row, V &) const;
  template <class V> void SetRow(int row, const V &);
  // vector helpers used internally
  template <class V> bool CheckRow(const V &v) const { return m_data.CheckRow(v); }
  template <class V> bool CheckColumn(const V &v) const { return m_data.CheckColumn(v); }
  bool CheckBounds(const Matrix &M) const { return m_data.CheckBounds(M.m_data); }

  /// @name Comparison operators
  //@{
  bool operator==(const Matrix &) const;
  bool operator!=(const Matrix &M) const { return !(*this == M); }
  bool operator==(const T &) const;
  bool operator!=(const T &c) const { return !(*this == c); }
  //@}

  /// @name Additive operators
  //@{
  Matrix operator+(const Matrix &M) const
  {
    Matrix tmp(*this);
    tmp += M;
    return tmp;
  }
  Matrix operator-(const Matrix &M) const
  {
    Matrix tmp(*this);
    tmp -= M;
    return tmp;
  }
  Matrix &operator+=(const Matrix &);
  Matrix &operator-=(const Matrix &);

  Matrix operator-() const;
  //@}

  /// @name Multiplicative operators
  //@{
  /// "in-place" column multiply
  void CMultiply(const Vector<T> &, Vector<T> &) const;
  /// "in-place" row (transposed) multiply
  void RMultiply(const Vector<T> &, Vector<T> &) const;
  Matrix operator*(const Matrix &) const;
  Vector<T> operator*(const Vector<T> &) const;
  Matrix operator*(const T &c) const
  {
    Matrix tmp(*this);
    tmp *= c;
    return tmp;
  }
  Matrix &operator*=(const T &);

  Matrix operator/(const T &c) const
  {
    Matrix tmp(*this);
    tmp /= c;
    return tmp;
  }
  Matrix &operator/=(const T &);
  //@

  /// @name Other operations
  //@{
  Matrix Transpose() const;
  //@}

  Matrix Inverse() const;
  T Determinant() const;
};

template <class T> Matrix<T> &Matrix<T>::operator=(const T &c)
{
  std::fill(m_data.elements_begin(), m_data.elements_end(), c);
  return *this;
}

// ----------------------------------------------------------------------------
// Implementation of element-wise operations
// ----------------------------------------------------------------------------

template <class T> bool Matrix<T>::operator==(const Matrix &M) const
{
  if (!this->CheckBounds(M)) {
    throw DimensionException();
  }
  return std::equal(m_data.elements_begin(), m_data.elements_end(), M.m_data.elements_begin());
}

template <class T> bool Matrix<T>::operator==(const T &c) const
{
  return std::all_of(m_data.elements_begin(), m_data.elements_end(),
                     [&c](const auto &v) { return v == c; });
}

template <class T> Matrix<T> &Matrix<T>::operator+=(const Matrix &M)
{
  if (!this->CheckBounds(M)) {
    throw DimensionException();
  }
  std::transform(m_data.elements_begin(), m_data.elements_end(), M.m_data.elements_begin(),
                 m_data.elements_begin(), std::plus<>());
  return *this;
}

template <class T> Matrix<T> &Matrix<T>::operator-=(const Matrix &M)
{
  if (!this->CheckBounds(M)) {
    throw DimensionException();
  }
  std::transform(m_data.elements_begin(), m_data.elements_end(), M.m_data.elements_begin(),
                 m_data.elements_begin(), std::minus<>());
  return *this;
}

template <class T> Matrix<T> Matrix<T>::operator-() const
{
  Matrix tmp(*this);
  std::transform(tmp.m_data.elements_begin(), tmp.m_data.elements_end(),
                 tmp.m_data.elements_begin(), std::negate<>());
  return tmp;
}

template <class T> Matrix<T> &Matrix<T>::operator*=(const T &c)
{
  std::transform(m_data.elements_begin(), m_data.elements_end(), m_data.elements_begin(),
                 [&c](const T &v) { return v * c; });
  return *this;
}

template <class T> Matrix<T> &Matrix<T>::operator/=(const T &c)
{
  if (c == static_cast<T>(0)) {
    throw ZeroDivideException();
  }
  std::transform(m_data.elements_begin(), m_data.elements_end(), m_data.elements_begin(),
                 [&c](const T &v) { return v / c; });
  return *this;
}

// ----------------------------------------------------------------------------
// Implementation of row/column access
// ----------------------------------------------------------------------------

template <class T> template <class V> void Matrix<T>::GetColumn(int col, V &v) const
{
  if (col < MinCol() || col > MaxCol()) {
    throw std::out_of_range("Index out of range in Matrix::GetColumn");
  }
  if (!CheckColumn(v)) {
    throw DimensionException();
  }
  for (int i = MinRow(); i <= MaxRow(); ++i) {
    v[i] = (*this)(i, col);
  }
}

template <class T> template <class V> void Matrix<T>::SetColumn(int col, const V &v)
{
  if (col < MinCol() || col > MaxCol()) {
    throw std::out_of_range("Index out of range in Matrix::SetColumn");
  }
  if (!CheckColumn(v)) {
    throw DimensionException();
  }
  for (int i = MinRow(); i <= MaxRow(); ++i) {
    (*this)(i, col) = v[i];
  }
}

template <class T> template <class V> void Matrix<T>::GetRow(int row, V &v) const
{
  if (row < MinRow() || row > MaxRow()) {
    throw std::out_of_range("Index out of range in Matrix::GetRow");
  }
  if (!CheckRow(v)) {
    throw DimensionException();
  }
  for (int j = MinCol(); j <= MaxCol(); ++j) {
    v[j] = (*this)(row, j);
  }
}

template <class T> template <class V> void Matrix<T>::SetRow(int row, const V &v)
{
  if (row < MinRow() || row > MaxRow()) {
    throw std::out_of_range("Index out of range in Matrix::SetRow");
  }
  if (!CheckRow(v)) {
    throw DimensionException();
  }
  for (int j = MinCol(); j <= MaxCol(); ++j) {
    (*this)(row, j) = v[j];
  }
}

// ----------------------------------------------------------------------------
// Implementation of linear algebra concepts
// ----------------------------------------------------------------------------

template <class T> void Matrix<T>::CMultiply(const Vector<T> &in, Vector<T> &out) const
{
  if (!this->CheckRow(in) || !this->CheckColumn(out)) {
    throw DimensionException();
  }
  for (int i = MinRow(); i <= MaxRow(); ++i) {
    auto row = m_data.GetRowView(i);
    out[i] = std::inner_product(row.begin(), row.end(), in.begin(), T{0});
  }
}

template <class T> void Matrix<T>::RMultiply(const Vector<T> &in, Vector<T> &out) const
{
  if (!this->CheckColumn(in) || !this->CheckRow(out)) {
    throw DimensionException();
  }

  out = T{0};
  for (int i = MinRow(); i <= MaxRow(); ++i) {
    auto row = m_data.GetRowView(i);
    const T k = in[i];
    auto dst = out.begin();
    for (auto it = row.begin(); it != row.end(); ++it, ++dst) {
      *dst += (*it) * k;
    }
  }
}

template <class T> Vector<T> Matrix<T>::operator*(const Vector<T> &v) const
{
  if (!this->CheckRow(v)) {
    throw DimensionException();
  }
  Vector<T> tmp(MinRow(), MaxRow());
  CMultiply(v, tmp);
  return tmp;
}

template <class T> Matrix<T> Matrix<T>::operator*(const Matrix &M) const
{
  if (MinCol() != M.MinRow() || MaxCol() != M.MaxRow()) {
    throw DimensionException();
  }
  Matrix<T> tmp(MinRow(), MaxRow(), M.MinCol(), M.MaxCol());
  for (int i = MinRow(); i <= MaxRow(); ++i) {
    auto row = m_data.GetRowView(i);
    for (int j = M.MinCol(); j <= M.MaxCol(); ++j) {
      auto col = M.m_data.GetColumnView(j);
      tmp(i, j) = std::inner_product(row.begin(), row.end(), col.begin(), T{0});
    }
  }
  return tmp;
}

template <class T> Matrix<T> Matrix<T>::Transpose() const
{
  Matrix tmp(MinCol(), MaxCol(), MinRow(), MaxRow());

  for (int i = MinRow(); i <= MaxRow(); ++i) {
    auto src_row = m_data.GetRowView(i);
    auto dst_col = tmp.m_data.GetColumnView(i);

    auto src = src_row.begin();
    auto dst = dst_col.begin();

    for (; src != src_row.end(); ++src, ++dst) {
      *dst = *src;
    }
  }

  return tmp;
}

// ----------------------------------------------------------------------------
// Implementation of additional operations
// ----------------------------------------------------------------------------

template <class T> Matrix<T> Matrix<T>::Inverse() const
{
  if (!IsSquare()) {
    throw DimensionException();
  }
  const int rmin = MinRow();
  const int rmax = MaxRow();
  const int cmin = MinCol();
  const int cmax = MaxCol();

  Matrix copy(*this);
  Matrix inv(rmin, rmax, cmin, cmax);

  inv = T{0};

  // initialize inverse matrix and prescale row vectors
  for (int i = rmin; i <= rmax; ++i) {
    auto copy_row = copy.m_data.GetRowView(i);
    auto inv_row = inv.m_data.GetRowView(i);

    T max = maximize_function(copy_row, [](const T &v) { return abs(v); });
    if (max == T{0}) {
      throw SingularMatrixException();
    }
    const T scale = T{1} / max;
    for (auto &v : copy_row) {
      v *= scale;
    }
    inv_row[i] = scale;
  }

  for (int i = cmin; i <= cmax; ++i) {
    // find pivot row
    auto col_i = copy.m_data.GetColumnView(i);
    T max = abs(col_i[i]);
    int row = i;
    for (int j = i + 1; j <= rmax; ++j) {
      const T v = abs(col_i[j]);
      if (v > max) {
        max = v;
        row = j;
      }
    }

    if (max <= T{0}) {
      throw SingularMatrixException();
    }

    copy.SwitchRows(i, row);
    inv.SwitchRows(i, row);
    // scale pivot row
    T factor = T{1} / copy(i, i);
    auto copy_row = copy.m_data.GetRowView(i);
    auto inv_row = inv.m_data.GetRowView(i);
    auto copy_it = copy_row.begin();
    auto inv_it = inv_row.begin();
    for (; copy_it != copy_row.end(); ++copy_it, ++inv_it) {
      *copy_it *= factor;
      *inv_it *= factor;
    }

    // reduce other rows
    auto pivot_copy_row = copy.m_data.GetRowView(i);
    auto pivot_inv_row = inv.m_data.GetRowView(i);

    for (int j = rmin; j <= rmax; ++j) {
      if (j == i) {
        continue;
      }
      auto row_copy = copy.m_data.GetRowView(j);
      auto row_inv = inv.m_data.GetRowView(j);
      const T mult = row_copy[i];
      auto pivot_copy_it = pivot_copy_row.begin();
      auto pivot_inv_it = pivot_inv_row.begin();
      auto row_copy_it = row_copy.begin();
      auto row_inv_it = row_inv.begin();

      for (; pivot_copy_it != pivot_copy_row.end();
           ++pivot_copy_it, ++pivot_inv_it, ++row_copy_it, ++row_inv_it) {
        *row_copy_it -= (*pivot_copy_it) * mult;
        *row_inv_it -= (*pivot_inv_it) * mult;
      }
    }
  }

  return inv;
}

template <class T> T Matrix<T>::Determinant() const
{
  if (!IsSquare()) {
    throw DimensionException();
  }

  T factor = (T)1;
  Matrix M(*this);

  for (int row = MinRow(); row <= MaxRow(); row++) {

    // Experience (as of 3/22/99) suggests that, in the interest of
    // numerical stability, it might be best to do Gaussian
    // elimination with respect to the row (of those feasible)
    // whose entry has the largest absolute value.
    int swap_row = row;
    for (int i = row + 1; i <= MaxRow(); i++) {
      if (abs(M(i, row)) > abs(M(swap_row, row))) {
        swap_row = i;
      }
    }

    if (swap_row != row) {
      M.SwitchRows(row, swap_row);
      for (int j = MinCol(); j <= MaxCol(); j++) {
        M(row, j) *= (T)-1;
      }
    }

    if (M(row, row) == (T)0) {
      return (T)0;
    }

    // now do row operations to clear the row'th column
    // below the diagonal
    for (int row1 = row + 1; row1 <= MaxRow(); row1++) {
      factor = -M(row1, row) / M(row, row);
      for (int i = MinCol(); i <= MaxCol(); i++) {
        M(row1, i) += M(row, i) * factor;
      }
    }
  }

  // finally we multiply the diagonal elements
  T det = (T)1;
  for (int row = MinRow(); row <= MaxRow(); row++) {
    det *= M(row, row);
  }
  return det;
}

// ----------------------------------------------------------------------------
// Implementation of operators
// ----------------------------------------------------------------------------

template <class T> Vector<T> operator*(const Vector<T> &v, const Matrix<T> &M)
{
  if (!M.CheckColumn(v)) {
    throw DimensionException();
  }
  Vector<T> tmp(M.MinCol(), M.MaxCol());
  M.RMultiply(v, tmp);
  return tmp;
}

// ----------------------------------------------------------------------------
// Explicit instantiations
// ----------------------------------------------------------------------------

extern template class Matrix<int>;
extern template class Matrix<double>;
extern template class Matrix<Integer>;
extern template class Matrix<Rational>;

} // end namespace Gambit

#endif // GAMBIT_CORE_MATRIX_H
