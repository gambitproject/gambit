//#
//# FILE: gsmatrix.h -- Implementation of square matrices
//#
//# @(#)gsmatrix.h	1.4 3/30/96
//#

#ifndef GSMATRIX_H
#define GSMATRIX_H

#include "gmatrix.h"

template <class T> class gSquareMatrix : public gMatrix<T>   {
  public:
    gSquareMatrix(void);
    gSquareMatrix(int size);
    gSquareMatrix(const       gMatrix<T> &M);
    gSquareMatrix(const gSquareMatrix<T> &M);
    virtual ~gSquareMatrix();

    gSquareMatrix<T> &operator=(const gSquareMatrix<T> &);

      // TRANSPOSE, INVERSION, DETERMINANT
    gSquareMatrix<T>  Transpose()       const;
    gSquareMatrix<T>  Invert(void)      const;
    T                 Determinant(void) const;
};

template <class T> gOutput& operator<<(gOutput &to, const gSquareMatrix<T> &M);

#endif    // GSMATRIX_H
