//
// FILE: homotopy.h -- Calls hompack code
//
// $Id$
//

#ifndef HOMOTOPY_H
#define HOMOTOPY_H

#include "algutils.h"
#include "glist.h"

#include "nfg.h"
#include "mixedsol.h"

class HomQreParams : public FuncMinParams {
public:
  int powLam;
  double minLam, maxLam, delLam;
  bool fullGraph;
  gOutput *pxifile;
  
  HomQreParams(gStatus & = gstatus);
  HomQreParams(gOutput &out, gOutput &pxi,gStatus & = gstatus);
};


void HomQre(const Nfg &, HomQreParams &,
	   const MixedProfile<gNumber> &, gList<MixedSolution> &,
	   long &nevals, long &nits);



template <class T> void display_vector(const gVector<T> p,char * msg, int n,T ptag);
template <class T> T my_log(T x,T eps);

#ifdef UNUSED
template <class T> void rhojact( const gVector<T> &A,const NFSupport &supp, 
             const gMatrix<T> &row_payoffs,
             const gMatrix<T> &col_payoffs, const T lambda,
             const gVector<T> &X, gVector<T> &V, int K );
template <class T> T eu(const gVector<T> p,const gMatrix<T> M,
              const int indicator,const int n);
#endif
template <class T> T tl(T x);

template <class T> void rho( const NFSupport &supp, const gVector<T> &A, 
          const T lambda, const gVector<T> &X, gVector<T> &V   );

template <class T> void rhojac(const NFSupport &supp,  const gVector<T> &A,  
             const T lambda, const gVector<T> &X, gVector<T> &V, int K );

template <class T> void F( const gVector<T> &X, gVector<T> &V );
template <class T> void Fjac( const gVector<T> &X, gVector<T> &V, int K );

void echo_payoffs(const NFSupport &supp);
#endif // HOMOTOPY_H


