//#
//# FILE: lemke.h -- Interface to Lemke solution module
//#
//# $Id$
//#

#ifndef LEMKE_H
#define LEMKE_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "normal.h"

class LemkeParams     {
  public:
    int dup_strat, plev, nequilib, maxdepth;
    gString outfile, errfile;
    
    LemkeParams(void);
};

class LemkeSolver  {
  private:
    const BaseNormalForm &nf;
    LemkeParams params;
    int npivots;
    gRational time;

  public:
    LemkeSolver(const BaseNormalForm &N, const LemkeParams &p) 
      : nf(N), params(p)   { }
    ~LemkeSolver()   { }

    int Lemke(void);
    
    int NumPivots(void) const    { return npivots; }
    gRational Time(void) const    { return time; }

    LemkeParams &Parameters(void)   { return params; }
};


#endif    // LEMKE_H



