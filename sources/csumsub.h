//
// FILE: csumsub.h -- Solve efg by LP on nfg
//
// $Id$
//

#ifndef CSUMSUB_H
#define CSUMSUB_H

#include "subsolve.h"
#include "nfgcsum.h"

int ZSum(const EFSupport &, const ZSumParams &, const gArray<gNumber> &,
	 gList<BehavSolution> &, int &npivots, double &time);



#endif    // CSUMSUB_H

