//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to Lyapunov function minimization for computing equilibrium
//

#ifndef ALGLIAP_H
#define ALGLIAP_H

#include "game/efg.h"
#include "nash/behavsol.h"
#include "game/nfg.h"
#include "nash/mixedsol.h"

bool LiapEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LiapNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LiapNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGLIAP_H
