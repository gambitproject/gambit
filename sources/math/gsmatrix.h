//
// FILE: gsmatrix.h -- Implementation of square matrices
//
// $Id$
//

#ifndef GSMATRIX_H
#define GSMATRIX_H

#include "math/gmatrix.h"

template <class T> class gSquareMatrix : public gMatrix<T>   {
public:
  class MatrixSingular : public gException {
  public:
    virtual ~MatrixSingular();
    gText Description(void) const;
  };

  gSquareMatrix(void);
  gSquareMatrix(int size);
  gSquareMatrix(const gMatrix<T> &);
  gSquareMatrix(const gSquareMatrix<T> &);
  virtual ~gSquareMatrix();

  gSquareMatrix<T> &operator=(const gSquareMatrix<T> &);

  gSquareMatrix<T> Inverse(void) const;
  T Determinant(void) const;
};

template <class T> gOutput& operator<<(gOutput &to, const gSquareMatrix<T> &M);

#endif    // GSMATRIX_H
