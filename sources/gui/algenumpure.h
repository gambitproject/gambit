//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to pure-strategy equilibrium enumeration algorithms
//

#ifndef ALGENUMPURE_H
#define ALGENUMPURE_H

#include "game/efg.h"
#include "game/behavsol.h"
#include "game/nfg.h"
#include "game/mixedsol.h"

bool EnumPureEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool EnumPureNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool EnumPureNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGENUMPURE_H
