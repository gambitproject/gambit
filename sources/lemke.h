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
#include "rational.h"

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

//
// This is a dirty hack.  It returns the solutions from the last invocation
// of LemkeSolver::Lemke() as a block of pointers to profile vectors.
// The user is responsible for deleting the pointers after he is done with
// them.
//
template <class T> GetLemkeSolution(gBlock<gPVector<T> *> &);


#endif    // LEMKE_H



