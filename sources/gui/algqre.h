//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to quantal-response correspondence tracing
//

#ifndef ALGQRE_H
#define ALGQRE_H

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool QreEfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool QreNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool QreNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGQRE_H
