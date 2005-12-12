//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of square matrices
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

#ifndef GSMATRIX_H
#define GSMATRIX_H

#include "gmatrix.h"

class gbtSingularMatrixException : public gbtException {
public:
  virtual ~gbtSingularMatrixException() { }
  std::string GetDescription(void) const
    { return "Attempted to invert a singular matrix"; }
};

template <class T> class gbtSquareMatrix : public gbtMatrix<T>   {
public:
  gbtSquareMatrix(void);
  gbtSquareMatrix(int size);
  gbtSquareMatrix(const gbtMatrix<T> &);
  gbtSquareMatrix(const gbtSquareMatrix<T> &);
  virtual ~gbtSquareMatrix();

  gbtSquareMatrix<T> &operator=(const gbtSquareMatrix<T> &);

  gbtSquareMatrix<T> Inverse(void) const;
  T Determinant(void) const;
};

#endif    // GSMATRIX_H
