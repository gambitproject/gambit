//
// FILE: psnesub.h -- Solve efg by subgames in pure nfg strategies
//
// $Id$
//

#ifndef PSNESUB_H
#define PSNESUB_H

#include "nfgpure.h"
#include "behavsol.h"

int EnumPureNfg(const EFSupport &, gList<BehavSolution> &, double &time);
	     
#endif   // PSNESUB_H

