//
// FILE: lapack.h -- Hompack code, translated by David Bustos, 6/25/99
//
// $Id$
//

#ifndef LAPACK_H
#define LAPACK_H

// Includes //////////////////////////////////////////////////////////////
#include "gvector.h"
#include "gmatrix.h"

// Prototypes ////////////////////////////////////////////////////////////

template <class T> class gLapack {
public: 
  gLapack(void);
  virtual ~gLapack();

  void dgeqpf(gMatrix<T> &A, gArray<int> &jpvt, gVector<T> &tau, int &info);
  void dgeqr2(gMatrix<T> &A, gArray<T> &tau, int &info);
  void dlarfg(T &alpha, gVector<T> &X, T &tau);
  void dormqr(char side, char trans, int k, gMatrix<T> &A,
            gVector<T> &tau, gMatrix<T> &C, int &info);
  void dorm2r(char side, char trans, int k, gMatrix<T> &A,
            gVector<T> &tau, gMatrix<T> &C, int &info);
  void dlarf(char side, gVector<T> &V, T tau, gMatrix<T> &C);
  void xerbla(char *srname, int info);
};
#endif // LAPACK




