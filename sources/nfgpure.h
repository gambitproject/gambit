//
// FILE: nfgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef NFGPURE_H
#define NFGPURE_H

#include "nfg.h"
#include "mixed.h"
#include "mixedsol.h"
#include "glist.h"


int FindPureNash(const Nfg &N, const NFSupport &, gList<MixedSolution> &);


#endif    // NFGPURE_H


