//#
//# FILE: liap.h -- Interface to Liap solution module
//#
//# $Id$
//#

#ifndef LIAP_H
#define LIAP_H

#include "normal.h"

class LiapParams     {
  public:
    int plev,number;
    gString outfile, errfile;
    
    LiapParams(void) : number(0), plev(2)   { }
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



