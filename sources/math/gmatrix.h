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

#include "base/grarray.h"
#include "gvector.h"

template <class T> class gMatrix : public gRectArray<T>  {
  public:
    class DivideByZero : public gException  {
      public:
        virtual ~DivideByZero()   { }
        gText Description(void) const;
    };

       // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
    gMatrix(void);
    gMatrix(unsigned int rows, unsigned int cols);
    gMatrix(unsigned int rows, unsigned int cols, int minrows);
    gMatrix(int rl, int rh, int cl, int ch);
    gMatrix(const gMatrix<T> &);
    virtual ~gMatrix();

    gMatrix<T> &operator=(const gMatrix<T> &);
    gMatrix<T> &operator=(const T &);
    gMatrix<T> operator-(void);

       // ADDITIVE OPERATORS
    gMatrix<T> operator+(const gMatrix<T> &) const;
    gMatrix<T> operator-(const gMatrix<T> &) const;
    gMatrix<T> &operator+=(const gMatrix<T> &);
    gMatrix<T> &operator-=(const gMatrix<T> &);


       // MULTIPLICATIVE OPERATORS
       // "in-place" column multiply
    void CMultiply(const gVector<T> &, gVector<T> &) const;
       // "in-place" row (transposed) multiply
    void RMultiply(const gVector<T> &, gVector<T> &) const;
    gMatrix<T> operator*(const gMatrix<T> &) const;
    gVector<T> operator*(const gVector<T> &) const;
    gMatrix<T> operator*(const T &) const;
    gMatrix<T> &operator*=(const gMatrix<T> &);
    gMatrix<T> &operator*=(const T &);

    gMatrix<T> operator/(const T &) const;
    gMatrix<T> &operator/=(const T &);

       // KRONECKER PRODUCT
    gMatrix<T> operator&(const gMatrix<T> &) const;

      // TRANSPOSE
    gMatrix<T>       Transpose()         const;

       // COMPARISON OPERATORS
    bool operator==(const gMatrix<T> &) const;
    bool operator!=(const gMatrix<T> &) const;
    bool operator==(const T &) const;
    bool operator!=(const T &) const;

       // INFORMATION
    gVector<T> Row   (const int&) const;
    gVector<T> Column(const int&) const;

  void MakeIdent(void);  // set matrix to identity matrix
  void Pivot(int, int);
};

template <class T> gVector<T> operator*(const gVector<T> &, const gMatrix<T> &);
template <class T> gOutput &operator<<(gOutput &, const gMatrix<T> &);

#endif     // GMATRIX_H




