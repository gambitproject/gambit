//#
//# FILE: simpdiv.h -- Interface to Simpdiv solution module
//#
//# $Id$
//#

#ifndef SIMPDIV_H
#define SIMPDIV_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "normal.h"
#include "rational.h"

class SimpdivParams     {
  public:
    int plev, number, ndivs, leash;
    gString outfile, errfile;
    
    SimpdivParams(void);
};

class SimpdivSolver  {
  private:
    const BaseNormalForm &nf;
    SimpdivParams params;
    int nevals;
    gRational time;

  public:
    SimpdivSolver(const BaseNormalForm &N, const SimpdivParams &p) 
      : nf(N), params(p)   { }
    ~SimpdivSolver()   { }

    int Simpdiv(void);
    
    int NumEvals(void) const    { return nevals; }
    gRational Time(void) const   { return time; }

    SimpdivParams &Parameters(void)   { return params; }
};


#endif    // SIMPDIV_H



