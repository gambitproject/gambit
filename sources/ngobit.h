//#
//# FILE: gobit.h -- Interface to Gobit solution module
//#
//# @(#)gobit.h	1.1 11/22/94
//#

#ifndef GOBIT_H
#define GOBIT_H

#include "normal.h"

class GobitParams     {
  public:
    int plev,number;
    gString outfile, errfile;
    
    GobitParams(void) :  plev(0),number(1)   { }
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



