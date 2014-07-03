//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/sqmatrix.h
// Implementation of square matrices
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

#ifndef LIBGAMBIT_SQMATRIX_H
#define LIBGAMBIT_SQMATRIX_H

#include "matrix.h"


namespace Gambit {

class SingularMatrixException : public Exception {
public:
  virtual ~SingularMatrixException() throw() { }
  const char *what(void) const throw() { return "Attempted to invert a singular matrix"; }
};

template <class T> class SquareMatrix : public Matrix<T>   {
public:
  SquareMatrix(void);
  SquareMatrix(int size);
  SquareMatrix(const Matrix<T> &);
  SquareMatrix(const SquareMatrix<T> &);
  virtual ~SquareMatrix();

  SquareMatrix<T> &operator=(const SquareMatrix<T> &);

  SquareMatrix<T> Inverse(void) const;
  T Determinant(void) const;
};

}  // end namespace Gambit

#endif   // LIBGAMBIT_SQMATRIX_H
