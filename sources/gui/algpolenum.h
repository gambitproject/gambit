//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to equilibrium enumeration via polynomial solver algorithms
//

#ifndef ALGPOLENUM_H
#define ALGPOLENUM_H

#include "game/efg.h"
#include "nash/behavsol.h"
#include "game/nfg.h"
#include "nash/mixedsol.h"

bool PolEnumEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool PolEnumNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool PolEnumNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGPOLENUM_H
