//#
//# FILE: lemke.h -- Interface to Lemke solution module
//#
//# $Id$
//#

#ifndef LEMKE_H
#define LEMKE_H

#include "normal.h"

class LemkeParams     {
  public:
    int dup_strat;
    int plev;
    gString outfile, errfile;
    
    LemkeParams(void) : dup_strat(0), plev(0)   { }
};

class LemkeSolver  {
  private:
    const BaseNormalForm &nf;
    LemkeParams params;
    int npivots;

  public:
    LemkeSolver(const BaseNormalForm &N, const LemkeParams &p) 
      : nf(N), params(p)   { }
    ~LemkeSolver()   { }

    int Lemke(void);
    
    int NumPivots(void) const    { return npivots; }

    LemkeParams &Parameters(void)   { return params; }
};


#endif    // LEMKE_H



