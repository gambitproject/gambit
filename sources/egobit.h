//
// FILE: egobit.h -- Interface to extensive form Gobit solution module
//
// @(#)egobit.h	2.1 3/24/97
//

#ifndef EGOBIT_H
#define EGOBIT_H

#include "gambitio.h"
#include "gstatus.h"
#include "glist.h"

#include "efg.h"
#include "behav.h"
#include "behavsol.h"

class EFGobitParams   {
  public:
    int trace, powLam, maxits1, maxitsN;
    double minLam, maxLam, delLam, tol1, tolN;
    bool fullGraph;
    gOutput *tracefile, *pxifile;
    gStatus &status;

    EFGobitParams(gStatus & = gstatus);
    EFGobitParams(gOutput &out, gOutput &pxi, gStatus & = gstatus);
};


void Gobit(const Efg &, EFGobitParams &,
	   const BehavProfile<gNumber> &, gList<BehavSolution<double> > &,
	   long &nevals, long &nits);


#endif    // NGOBIT_H



