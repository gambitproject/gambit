//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to quantal-response correspondence grid search
//

#ifndef ALGQREGRID_H
#define ALGQREGRID_H

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "mixedsol.h"

bool QreGridNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool QreGridNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGQREGRID_H
