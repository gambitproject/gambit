//
// FILE: ngobit.h -- Interface to normal form Gobit solution module
//
// $Id$
//

#ifndef NGOBIT_H
#define NGOBIT_H

#include "gambitio.h"
#include "gstatus.h"
#include "glist.h"

#include "nfg.h"
#include "mixed.h"

class NFGobitParams  {
  public:
    int trace, powLam, maxits1, maxitsN;
    double minLam, maxLam, delLam, tol1, tolN;
    bool fullGraph;
    gOutput *tracefile, *pxifile;
    gStatus &status;

    NFGobitParams(gStatus & = gstatus);
    NFGobitParams(gOutput &out, gOutput &pxi, gStatus & = gstatus);
};


void Gobit(const Nfg<double> &, NFGobitParams &,
	   const MixedProfile<double> &, gList<MixedProfile<double> > &,
	   long &nevals, long &nits);


#endif    // NGOBIT_H



