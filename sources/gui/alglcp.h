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

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool LcpEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LcpNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool LcpNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGENUMMIXED_H
