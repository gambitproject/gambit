//
// FILE: enumsub.h -- Solve efg by enumeration in nfg
//
// $Id$
//

#ifndef ENUMSUB_H
#define ENUMSUB_H

#include "enum.h"
#include "behavsol.h"

int Enum(const EFSupport &, const EnumParams &, const gArray<gNumber> &,
	 gList<BehavSolution> &, long &npivots, double &time);

#endif   // ENUMSUB_H


