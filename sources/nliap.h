//
// FILE: nliap.h -- Interface to normal form Liap solution module
//
// $Id$
//

#ifndef NLIAP_H
#define NLIAP_H

#include "base/base.h"
#include "algutils.h"

#include "game/nfg.h"
#include "game/mixed.h"
#include "game/mixedsol.h"

class NFLiapParams : public FuncMinParams  {
public:
  int nTries;
  
  NFLiapParams(void);
};


bool Liap(const Nfg &, NFLiapParams &,
	  const MixedProfile<gNumber> &, gList<MixedSolution> &, gStatus &,
	  long &nevals, long &niters);

#endif   // NLIAP_H

