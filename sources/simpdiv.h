//#
//# FILE: simpdiv.h -- Interface to Simpdiv solution module
//#
//# @(#)simpdiv.h	1.1 11/22/94
//#

#ifndef SIMPDIV_H
#define SIMPDIV_H

#include "normal.h"

class SimpdivParams     {
  public:
    int plev,number,ndivs,leashlength;
    gString outfile, errfile;
    
    SimpdivParams(void) : number(1), plev(0),ndivs(20),leashlength(0)   { }
};

class SimpdivSolver  {
  private:
    const BaseNormalForm &nf;
    SimpdivParams params;
    int nevals;

  public:
    SimpdivSolver(const BaseNormalForm &N, const SimpdivParams &p) 
      : nf(N), params(p)   { }
    ~SimpdivSolver()   { }

    int Simpdiv(void);
    
    int NumEvals(void) const    { return nevals; }

    SimpdivParams &Parameters(void)   { return params; }
};


#endif    // SIMPDIV_H



