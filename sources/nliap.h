//
// FILE: nliap.h -- Interface to normal form Liap solution module
//
// $Id$
//

#ifndef NLIAP_H
#define NLIAP_H

#include "gambitio.h"
#include "gstatus.h"
#include "glist.h"

#include "nfg.h"
#include "mixed.h"

class NFLiapParams   {
  public:
    int trace, maxits1, maxitsN;
    double tol1, tolN;
    gOutput *tracefile;
    gStatus &status;

    NFLiapParams(gStatus & = gstatus);
};


bool Liap(const Nfg<double> &, NFLiapParams &,
	  const MixedProfile<double> &, gList<MixedProfile<double> > &,
	  long &nevals, long &niters);

#endif   // NLIAP_H

