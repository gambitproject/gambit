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

#include "game/efg.h"
#include "nash/behavsol.h"
#include "game/nfg.h"
#include "nash/mixedsol.h"

bool SimpdivNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool SimpdivNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGSIMPDIV_H
