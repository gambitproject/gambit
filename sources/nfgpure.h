//#
//# FILE: purenash.h -- Interface to pure-strategy Nash equilibrium finder
//#
//# $Id$
//#

#ifndef PURENASH_H
#define PURENASH_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "normal.h"
#include "rational.h"

class PureNashParams   {
  public:
    int number, plev;
    gString outfile, errfile;

    PureNashParams(void);
};

class PureNashSolver   {
  private:
    const BaseNormalForm &nf;
    PureNashParams params;
    gRational time;

  public:
    PureNashSolver(const BaseNormalForm &N, const PureNashParams &p)
      : nf(N), params(p)   { }
    ~PureNashSolver()   { }
  
    int PureNash(void);

    gRational Time(void) const    { return time; }

    PureNashParams &Parameters(void)   { return params; }
};

/*********************************************/



#endif    // PURENASH_H


