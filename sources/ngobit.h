//#
//# FILE: gobit.h -- Interface to Gobit solution module
//#
//# $Id$
//#

#ifndef GOBIT_H
#define GOBIT_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "normal.h"

class GobitParams     {
  public:
    int plev, nequilib, type, maxitsDFP, maxitsBrent;
    gRational minLam, maxLam, delLam, tolDFP, tolBrent;
    gString outfile, errfile;
    
    GobitParams(void);
};

class GobitSolver  {
  private:
    const BaseNormalForm &nf;
    GobitParams params;
    int nevals;

  public:
    GobitSolver(const BaseNormalForm &N, const GobitParams &p) 
      : nf(N), params(p)   { }
    ~GobitSolver()   { }

    int Gobit(void);
    
    int NumEvals(void) const    { return nevals; }

    GobitParams &Parameters(void)   { return params; }
};

#endif    // GOBIT_H



