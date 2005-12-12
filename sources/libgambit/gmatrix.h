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

#ifndef GMATRIX_H
#define GMATRIX_H

#include "grarray.h"
#include "gvector.h"

template <class T> class gbtMatrix : public gbtRectArray<T>  {
  public:
       // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
    gbtMatrix(void);
    gbtMatrix(unsigned int rows, unsigned int cols);
    gbtMatrix(unsigned int rows, unsigned int cols, int minrows);
    gbtMatrix(int rl, int rh, int cl, int ch);
    gbtMatrix(const gbtMatrix<T> &);
    virtual ~gbtMatrix();

    gbtMatrix<T> &operator=(const gbtMatrix<T> &);
    gbtMatrix<T> &operator=(const T &);
    gbtMatrix<T> operator-(void);

       // ADDITIVE OPERATORS
    gbtMatrix<T> operator+(const gbtMatrix<T> &) const;
    gbtMatrix<T> operator-(const gbtMatrix<T> &) const;
    gbtMatrix<T> &operator+=(const gbtMatrix<T> &);
    gbtMatrix<T> &operator-=(const gbtMatrix<T> &);


       // MULTIPLICATIVE OPERATORS
       // "in-place" column multiply
    void CMultiply(const gbtVector<T> &, gbtVector<T> &) const;
       // "in-place" row (transposed) multiply
    void RMultiply(const gbtVector<T> &, gbtVector<T> &) const;
    gbtMatrix<T> operator*(const gbtMatrix<T> &) const;
    gbtVector<T> operator*(const gbtVector<T> &) const;
    gbtMatrix<T> operator*(const T &) const;
    gbtMatrix<T> &operator*=(const gbtMatrix<T> &);
    gbtMatrix<T> &operator*=(const T &);

    gbtMatrix<T> operator/(const T &) const;
    gbtMatrix<T> &operator/=(const T &);

       // KRONECKER PRODUCT
    gbtMatrix<T> operator&(const gbtMatrix<T> &) const;

      // TRANSPOSE
    gbtMatrix<T>       Transpose()         const;

       // COMPARISON OPERATORS
    bool operator==(const gbtMatrix<T> &) const;
    bool operator!=(const gbtMatrix<T> &) const;
    bool operator==(const T &) const;
    bool operator!=(const T &) const;

       // INFORMATION
    gbtVector<T> Row   (const int&) const;
    gbtVector<T> Column(const int&) const;

  void MakeIdent(void);  // set matrix to identity matrix
  void Pivot(int, int);
};

template <class T> gbtVector<T> operator*(const gbtVector<T> &, const gbtMatrix<T> &);
#endif     // GMATRIX_H




