//
// FILE: nfgcsum.h -- Interface to Constant Sum Game Solution Solver
//
// $Id$
//

#ifndef NFGCSUM_H
#define NFGCSUM_H

#include "nfg.h"
#include "glist.h"
#include "gstatus.h"
#include "mixedsol.h"

class ZSumParams     {
  public:
    int trace, stopAfter;
    Precision precision;
    gOutput *tracefile;
    gStatus &status;
    
    ZSumParams(gStatus &status_ = gstatus);
};

int ZSum(const NFSupport &, const ZSumParams &, const gArray<gNumber> &,
	 gList<MixedSolution> &, int &npivots, double &time);

#endif    // NFGCSUM_H



