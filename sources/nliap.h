//
// FILE: nliap.h -- Interface to normal form Liap solution module
//
// $Id$
//

#ifndef NLIAP_H
#define NLIAP_H

#include "gstream.h"
#include "gstatus.h"
#include "glist.h"

#include "nfg.h"
#include "mixed.h"
#include "mixedsol.h"

class NFLiapParams   {
  public:
    int trace, nTries, stopAfter, maxits1, maxitsN;
    double tol1, tolN;
    gOutput *tracefile;
    gStatus &status;

    NFLiapParams(gStatus & = gstatus);
};


bool Liap(const Nfg &, NFLiapParams &,
	  const MixedProfile<gNumber> &, gList<MixedSolution> &,
	  long &nevals, long &niters);

#endif   // NLIAP_H

