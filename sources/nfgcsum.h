//
// FILE: nfgcsum.h -- Interface to Constant Sum Game Solution Solver
//
// $Id$
//

#ifndef NFGCSUM_H
#define NFGCSUM_H

#include "nfg.h"
#include "glist.h"
#include "algutils.h"
#include "mixedsol.h"

class ZSumParams : public AlgParams    {
public:
  ZSumParams(void);
};

int ZSum(const NFSupport &, const ZSumParams &,
	 gList<MixedSolution> &, gStatus &, int &npivots, double &time);

#endif    // NFGCSUM_H



