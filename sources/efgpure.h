//
// FILE: efgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef EFGPURE_H
#define EFGPURE_H

#include "efg.h"
#include "behavsol.h"
#include "glist.h"

#include "subsolve.h"


int EnumPure(const EFSupport &, gList<BehavSolution> &, double &time);


#endif    // EFGPURE_H


