//
// FILE: simpsub.h -- Solve efg by simpdiv on nfg
//
// $Id$
//

#ifndef SIMPSUB_H
#define SIMPSUB_H

#include "simpdiv.h"
#include "subsolve.h"

int Simpdiv(const EFSupport &, const SimpdivParams &, const gArray<gNumber> &,
	    gList<BehavSolution> &, int &nevals, int &niters, double &time);


#endif   // SIMPSUB_H

