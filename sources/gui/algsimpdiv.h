//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to simplicial subdivision equilibrium computation algorithm
//

#ifndef ALGSIMPDIV_H
#define ALGSIMPDIV_H

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool SimpdivNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool SimpdivNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGSIMPDIV_H
