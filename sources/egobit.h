//#
//# FILE: egobit.h -- Interface to Gobit solution module
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
    const BaseExtForm &ef;
    ExtGobitParams params;
    int nevals;
    gRational time;

  public:
    ExtGobitSolver(const BaseExtForm &E, const ExtGobitParams &p) 
      : ef(E), params(p)   { }
    ~ExtGobitSolver()   { }

    int Gobit(void);
    
    int NumEvals(void) const    { return nevals; }
    gRational Time(void) const   { return time; }

    ExtGobitParams &Parameters(void)   { return params; }
};

#endif    // EGOBIT_H






