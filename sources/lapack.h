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
void dgeqpf(gMatrix<double> &A, gArray<int> &jpvt, gVector<double> &tau, int &info);

void dgeqr2(gMatrix<double> &A, gArray<double> &tau, int &info);

void dlarfg(double &alpha, gVector<double> &X, double &tau);

void dormqr(char side, char trans, int k, gMatrix<double> &A,
            gVector<double> &tau, gMatrix<double> &C, int &info);

void dorm2r(char side, char trans, int k, gMatrix<double> &A,
            gVector<double> &tau, gMatrix<double> &C, int &info);

void dlarf(char side, gVector<double> &V, double tau, gMatrix<double> &C);

void xerbla(char *srname, int info);

#endif




