//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to linear-programming equilibrium computation algorithms
//

#ifndef ALGLP_H
#define ALGLP_H

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool LpEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LpNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LpNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGLP_H
