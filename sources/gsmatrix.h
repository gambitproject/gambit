//#
//# FILE: gsmatrix.h -- Implementation of square matrices
//#
//# $Id$
//#

#ifndef GSMATRIX_H
#define GSMATRIX_H

#include "gmatrix.h"

template <class T> class gSquareMatrix : public gMatrix<T>   {
  private:
    gSquareMatrix<T> GetSubMatrix(const gArray<int> &, const gArray<int> &) const;

  public:
    gSquareMatrix(void);
    gSquareMatrix(int size);
    gSquareMatrix(const gSquareMatrix<T> &M);
    virtual ~gSquareMatrix();

    gSquareMatrix<T> &operator=(const gSquareMatrix<T> &);

    gSquareMatrix<T> Invert(void) const;
    T Determinant(void) const;
};

#endif    // GSMATRIX_H
