//#
//# FILE: gobit.h -- Interface to Gobit solution module
//#
//# $Id$
//#

#ifndef NGOBIT_H
#define NGOBIT_H

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
    gRational time;

  public:
    GobitSolver(const BaseNormalForm &N, const GobitParams &p) 
      : nf(N), params(p)   { }
    ~GobitSolver()   { }

    int Gobit(void);
    
    int NumEvals(void) const    { return nevals; }
    gRational Time(void) const   { return time; }

    GobitParams &Parameters(void)   { return params; }
};

#endif    // NGOBIT_H



