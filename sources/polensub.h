//
// FILE: polensub.h -- Solve efg by polynomial enumeration in nfg
//
// $Id$
//

#ifndef POLENSUB_H
#define POLENSUB_H

#include "polenum.h"
#include "behavsol.h"

int PolEnum(const EFSupport &, const PolEnumParams &,
	    gList<BehavSolution> &, long &nevals, double &time);

#endif   // POLENSUB_H


