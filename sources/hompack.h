//
// FILE: hompack.h -- Hompack code, translated by David Bustos, 6/25/99
//
// $Id$
//

#ifndef HOMPACK_H
#define HOMPACK_H

// Includes //////////////////////////////////////////////////////////////
#include "garray.h"
#include "gvector.h"
#include "gmatrix.h"
#include "stdio.h"
#include "homotopy.h" // needed for HomQreParams

// User Functions ////////////////////////////////////////////////////////
template <class T> void F( const gVector<T> &X, gVector<T> &V );

template <class T> void Fjac( const gVector<T> &X, gVector<T> &V, int K );

template <class T> void rho( const gVector<T> &A, const T lambda,
          const gVector<T> &X, gVector<T> &V);

template <class T> void rhojac( const gVector<T> &A, const T lambda,
             const gVector<T> &X, gVector<T> &V, int K );


// Prototypes ////////////////////////////////////////////////////////////
template <class T> void fixpnf(
	     const NFSupport &supp,
             HomQreParams &params, 
             gList<MixedSolution> &solutions,
	     int N, gVector<T> &Y, int &flag, T rel_arc_err,
             T abs_arc_err, T rel_ans_err, T abs_ans_err,
             bool trace, gVector<T> &A, gArray<T> &stepsize_params,
             int &jeval_num, T &arclength,T max_lambda,
             bool poly_switch);

template <class T> void stepnf( int N,
             int &jeval_num,
             int &flag,
             bool &start,       bool &crash,
             T &old_step,  T &step_size,
             T &rel_err,   T &abs_err,
             T &arclength,
             gVector<T> &Y,        gVector<T> &Ytan,
             gVector<T> &Y_old,    gVector<T> &Ytan_old,
             const gVector<T> &A,
             const gArray<T> &stepsize_params
           );

template <class T> void rootnf( int N,
             int &jevalcount, int &flag,
             T relerr,     T abserr,
             const gVector<T> &Y,
             gVector<T> &Ytan,
             gVector<T> &Y_old,
             gVector<T> &Ytan_old,
             const gVector<T> &A
           );

template <class T> void root( T &t, T &F_of_t,
           T &b, T &c,
           T relerr, T abserr,
           int &flag
         );
template <class T> void show_probs(char *msg,const gVector<T> &Y,NFSupport &supp);
template <class T> bool out_of_bounds(gVector<T> &W,int range);
template <class T> void build_prediction(gVector<T> &W,int n, gVector<T> &Y_old,
						gVector<T> &Ytan_old,gVector<T> &Y,
                  gVector<T> &Ytan,T old_step,T old_step_plus_step_size
                 );
template <class T> void tangnf( T &rholen,
             const gVector<T> &Y,
             gVector<T> &Ytan,
             const gVector<T> &Ytan_old,
             const gVector<T> &A,
             gVector<T> &Newton_step,
             int &jevalcount, int N, int &flag
           );

template <class T> T qofs(T f0, T fp0, T f1, T fp1, T dels, T s);

#endif // HOMPACK_H
