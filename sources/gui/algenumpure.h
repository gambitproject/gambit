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

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool EnumPureEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool EnumPureNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool EnumPureNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGENUMPURE_H
