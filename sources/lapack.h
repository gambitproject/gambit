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
template <class T> void dgeqpf(gMatrix<T> &A, gArray<int> &jpvt, gVector<T> &tau, int &info);

template <class T> void dgeqr2(gMatrix<T> &A, gArray<T> &tau, int &info);

template <class T> void dlarfg(T &alpha, gVector<T> &X, T &tau);

template <class T> void dormqr(char side, char trans, int k, gMatrix<T> &A,
            gVector<T> &tau, gMatrix<T> &C, int &info);

template <class T> void dorm2r(char side, char trans, int k, gMatrix<T> &A,
            gVector<T> &tau, gMatrix<T> &C, int &info);

template <class T> void dlarf(char side, gVector<T> &V, T tau, gMatrix<T> &C);

void xerbla(char *srname, int info);

#endif




