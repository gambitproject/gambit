//#
//# FILE: gobit.h -- Interface to Gobit solution module
//#
//# $Id$
//#

#ifndef EGOBIT_H
#define EGOBIT_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "extform.h"

class ExtGobitParams     {
  public:
    int plev, nequilib, type, maxitsPOW, maxitsBrent;
    gRational minLam, maxLam, delLam, tolPOW, tolBrent;                       
    gString outfile, errfile;
    
    ExtGobitParams(void);
};

class ExtGobitSolver  {
  private:
    const ExtForm<double> &nf;
    ExtGobitParams params;
    int nevals;

  public:
    ExtGobitSolver(const ExtForm<double> &N, const ExtGobitParams &p) 
      : nf(N), params(p)   { }
    ~ExtGobitSolver()   { }

    int Gobit(void);
    
    int NumEvals(void) const    { return nevals; }

    ExtGobitParams &Parameters(void)   { return params; }
};

#endif    // EGOBIT_H






