//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

template <class T> Vector<T> operator*(const Vector<T> &, const Matrix<T> &);

template <class T> class Matrix : public RectArray<T> {
  friend Vector<T> operator* <>(const Vector<T> &, const Matrix<T> &);

public:
  /// @name Lifecycle
  //@{
  Matrix();
  Matrix(unsigned int rows, unsigned int cols);
  Matrix(unsigned int rows, unsigned int cols, int minrows);
  Matrix(int rl, int rh, int cl, int ch);
  Matrix(const Matrix<T> &);
  ~Matrix() override;

  Matrix<T> &operator=(const Matrix<T> &);
  Matrix<T> &operator=(const T &);
  //@}

  /// @name Extracting rows and columns
  //@{
  bool IsSquare() const
  {
    return this->MinRow() == this->MinCol() && this->MaxRow() == this->MaxCol();
  }
  Vector<T> Row(int) const;
  Vector<T> Column(int) const;
  //@}

  /// @name Comparison operators
  //@{
  bool operator==(const Matrix<T> &) const;
  bool operator!=(const Matrix<T> &) const;
  bool operator==(const T &) const;
  bool operator!=(const T &) const;
  //@}

  /// @name Additive operators
  //@{
  Matrix<T> operator+(const Matrix<T> &) const;
  Matrix<T> operator-(const Matrix<T> &) const;
  Matrix<T> &operator+=(const Matrix<T> &);
  Matrix<T> &operator-=(const Matrix<T> &);

  Matrix<T> operator-();
  //@}

  /// @name Multiplicative operators
  //@{
  /// "in-place" column multiply
  void CMultiply(const Vector<T> &, Vector<T> &) const;
  /// "in-place" row (transposed) multiply
  void RMultiply(const Vector<T> &, Vector<T> &) const;
  Matrix<T> operator*(const Matrix<T> &) const;
  Vector<T> operator*(const Vector<T> &) const;
  Matrix<T> operator*(const T &) const;
  Matrix<T> &operator*=(const T &);

  Matrix<T> operator/(const T &) const;
  Matrix<T> &operator/=(const T &);
  //@

  /// @name Other operations
  //@{
  Matrix<T> Transpose() const;
  /// Set matrix to identity matrix
  void MakeIdent();
  void Pivot(int, int);
  //@}

  Matrix Inverse() const;
  T Determinant() const;
};

template <class T> Vector<T> operator*(const Vector<T> &, const Matrix<T> &);

} // end namespace Gambit

#endif // GAMBIT_CORE_MATRIX_H
