//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to LCP-based equilibrium computation algorithms
//

#ifndef ALGLCP_H
#define ALGLCP_H

#include "game/efg.h"
#include "game/behavsol.h"
#include "game/nfg.h"
#include "game/mixedsol.h"

bool LcpEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LcpNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LcpNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGENUMMIXED_H
