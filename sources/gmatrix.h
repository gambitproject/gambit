//#
//# FILE: gmatrix.h -- Implementation of matrix classes
//#
//# $Id$
//#

#ifndef GMATRIX_H
#define GMATRIX_H

#include "gambitio.h"
#include "grarray.h"
#include "gblock.h"
#include "gvector.h"
#include "gsmatrix.h"

template <class T> class gMatrix : public gRectArray<T>  {
  public:
       // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
    gMatrix(void);
    gMatrix(int rows, int cols);
    gMatrix(int rows, int cols, int minrows);
    gMatrix(int rl, int rh, int cl, int ch);
    gMatrix(const gMatrix<T> &);
    virtual ~gMatrix();

    gMatrix<T> &operator=(const gMatrix<T> &);

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
    friend gVector<T> operator*(const gVector<T> &, const gMatrix<T> &);
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

  void MakeIdent(void);  // set matrix to identity matrix
  void Pivot(int, int);
};

template <class T> gOutput &operator<<(gOutput &, const gMatrix<T> &);

#endif     // GMATRIX_H




