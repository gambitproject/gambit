//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to a matrix class
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

#ifndef LIBGAMBIT_MATRIX_H
#define LIBGAMBIT_MATRIX_H

#include "grarray.h"
#include "gvector.h"

namespace Gambit {

template <class T> class Matrix : public gbtRectArray<T>  {
public:
  /// @name Lifecycle
  //@{
  Matrix(void);
  Matrix(unsigned int rows, unsigned int cols);
  Matrix(unsigned int rows, unsigned int cols, int minrows);
  Matrix(int rl, int rh, int cl, int ch);
  Matrix(const Matrix<T> &);
  virtual ~Matrix();

  Matrix<T> &operator=(const Matrix<T> &);
  Matrix<T> &operator=(const T &);
  //@}

  /// @name Extracting rows and columns
  //@{
  gbtVector<T> Row(int) const;
  gbtVector<T> Column(int) const;
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

  Matrix<T> operator-(void);
  //@}


  /// @name Multiplicative operators
  //@{
  /// "in-place" column multiply
  void CMultiply(const gbtVector<T> &, gbtVector<T> &) const;
  /// "in-place" row (transposed) multiply
  void RMultiply(const gbtVector<T> &, gbtVector<T> &) const;
  Matrix<T> operator*(const Matrix<T> &) const;
  gbtVector<T> operator*(const gbtVector<T> &) const;
  Matrix<T> operator*(const T &) const;
  Matrix<T> &operator*=(const Matrix<T> &);
  Matrix<T> &operator*=(const T &);

  Matrix<T> operator/(const T &) const;
  Matrix<T> &operator/=(const T &);

  /// Kronecker product
  Matrix<T> operator&(const Matrix<T> &) const;
  //@

  /// @name Other operations
  //@{
  Matrix<T> Transpose(void) const;
  /// Set matrix to identity matrix
  void MakeIdent(void);  
  void Pivot(int, int);
  //@}
};

template <class T> 
gbtVector<T> operator*(const gbtVector<T> &, const Matrix<T> &);

} // end namespace Gambit

#endif  // LIBGAMBIT_MATRIX_H




