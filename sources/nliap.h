//#
//# FILE: liap.h -- Interface to Liap solution module
//#
//# $Id$
//#

#ifndef LIAP_H
#define LIAP_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "normal.h"
#include "gnumber.h"

class LiapParams     {
  public:
    int plev, ntries, nequilib, maxitsDFP, maxitsBrent;
    gRational tolDFP, tolBrent;
    gString outfile, errfile;
    
    LiapParams(void);
};

class LiapSolver  {
  private:
    const BaseNormalForm &nf;
    LiapParams params;
    int nevals,nits;

  public:
    LiapSolver(const BaseNormalForm &N, const LiapParams &p) 
      : nf(N), params(p),nevals(0),nits(0)   { }
    ~LiapSolver()   { }

    int Liap(void);
    
    int NumEvals(void) const    { return nevals; }
    int NumIters(void) const    { return nits; }

    LiapParams &Parameters(void)   { return params; }
};


#endif    // LIAP_H



