//#
//# FILE: liap.h -- Interface to Liap solution module
//#
//# $Id$
//#

#ifndef LIAP_H
#define LIAP_H

#include "normal.h"
#include "glist.h"

template <class T> class LiapParams     {
  public:
    int plev, ntries, nequilib, maxitsDFP, maxitsBrent;
    T tolDFP, tolBrent;
    gString outfile, errfile;
    
    LiapParams(void);
};

template <class T> class LiapSolver  {
  private:
    const NormalForm<T> &nf;
    LiapParams<T> params;
    int nevals,nits;
    gRational time;
    gList<gPVector<T> > solutions;

  public:
    LiapSolver(const NormalForm<T> &N, const LiapParams<T> &p); 

    int Liap(void);
    
    int NumEvals(void) const;
    int NumIters(void) const;
    gRational Time(void) const;

    LiapParams<T> &Parameters(void);

    const gList<gPVector<T> > &GetSolutions(void) const;
};


#endif    // LIAP_H



