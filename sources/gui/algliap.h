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

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool LiapEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LiapNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LiapNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGLIAP_H
