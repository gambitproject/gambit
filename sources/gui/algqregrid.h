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

#include "game/efg.h"
#include "game/behavsol.h"
#include "game/nfg.h"
#include "game/mixedsol.h"

bool QreGridNfg(wxWindow *, const EFSupport &, gList<BehavSolution> &);
bool QreGridNfg(wxWindow *, const NFSupport &, gList<MixedSolution> &);

#endif  // ALGQREGRID_H
