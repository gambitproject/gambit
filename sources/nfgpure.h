//
// FILE: nfgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef NFGPURE_H
#define NFGPURE_H

#include "nfg.h"
#include "mixedsol.h"
#include "glist.h"
#include "gstatus.h"

void FindPureNash(const NFSupport &, int, gStatus &, gList<MixedSolution> &);


#endif    // NFGPURE_H


