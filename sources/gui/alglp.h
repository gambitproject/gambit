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

#include "game/efg.h"
#include "game/behavsol.h"
#include "game/nfg.h"
#include "game/mixedsol.h"

bool LpEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LpNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LpNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGLP_H
