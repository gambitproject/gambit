//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to mixed-strategy equilibrium enumeration algorithms
//

#ifndef ALGENUMMIXED_H
#define ALGENUMMIXED_H

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool EnumMixedNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool EnumMixedNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGENUMMIXED_H
