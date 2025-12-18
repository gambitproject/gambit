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

  /// @name Extracting rows and columns
  //@{
  Vector<T> Row(int) const;
  Vector<T> Column(int) const;
  //@}

  /// @name Comparison operators
  //@{
  bool operator==(const Matrix &) const;
  bool operator!=(const Matrix &) const;
  bool operator==(const T &) const;
  bool operator!=(const T &) const;
  //@}

  /// @name Additive operators
  //@{
  Matrix operator+(const Matrix &) const;
  Matrix operator-(const Matrix &) const;
  Matrix &operator+=(const Matrix &);
  Matrix &operator-=(const Matrix &);

  Matrix operator-();
  //@}

  /// @name Multiplicative operators
  //@{
  /// "in-place" column multiply
  void CMultiply(const Vector<T> &, Vector<T> &) const;
  /// "in-place" row (transposed) multiply
  void RMultiply(const Vector<T> &, Vector<T> &) const;
  Matrix operator*(const Matrix &) const;
  Vector<T> operator*(const Vector<T> &) const;
  Matrix operator*(const T &) const;
  Matrix &operator*=(const T &);

  Matrix operator/(const T &) const;
  Matrix &operator/=(const T &);
  //@

  /// @name Other operations
  //@{
  Matrix Transpose() const;
  /// Set matrix to identity matrix
  void MakeIdent();
  void Pivot(int, int);
  //@}

  Matrix Inverse() const;
  T Determinant() const;
};

template <class T> Vector<T> operator*(const Vector<T> &, const Matrix<T> &);

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

} // end namespace Gambit

#endif // GAMBIT_CORE_MATRIX_H
