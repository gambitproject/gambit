//
// FILE: homotopy.h -- Calls hompack code
//
// $Id$
//

#ifndef HOMOTOPY_H
#define HOMOTOPY_H

#include "base/base.h"
#include "algutils.h"

#include "nfg.h"
#include "mixedsol.h"

class HomQreParams : public FuncMinParams {
public:
  int powLam;
  double minLam, maxLam, delLam;
  bool fullGraph;
  gOutput *pxifile;
  
  HomQreParams(void);
  HomQreParams(gOutput &out, gOutput &pxi);
};

void HomQre(const Nfg &, HomQreParams &,
	    const MixedProfile<gNumber> &, gList<MixedSolution> &, gStatus &,
	    long &nevals, long &nits);


template <class T> T my_log(T x,T eps);

#ifdef UNUSED
template <class T> void rhojact(const NFSupport &supp, const gVector<T> &A, 
             const T lambda, const gVector<T> &X, gVector<T> &V, int K );
template <class T> T eu(const gVector<T> p,const gMatrix<T> M,
              const int indicator,const int n);
#endif // UNUSED
template <class T> void rho( const NFSupport &supp, const gVector<T> &A, 
          const T lambda, const gVector<T> &X, gVector<T> &V   );

template <class T> void rhojac(const NFSupport &supp,  const gVector<T> &A,  
             const T lambda, const gVector<T> &X, gVector<T> &V, int K );

template <class T> void F( const gVector<T> &X, gVector<T> &V );
template <class T> void Fjac( const gVector<T> &X, gVector<T> &V, int K );

void echo_payoffs(const NFSupport &supp);
#endif // HOMOTOPY_H


