//
// FILE: nliap.h -- Interface to normal form Liap solution module
//
// $Id$
//

#ifndef NLIAP_H
#define NLIAP_H

#include "algutils.h"
#include "glist.h"


#include "nfg.h"
#include "mixed.h"
#include "mixedsol.h"

class NFLiapParams : public FuncMinParams  {
public:
  int nTries;
  
  NFLiapParams(gStatus & = gstatus);
};


bool Liap(const Nfg &, NFLiapParams &,
	  const MixedProfile<gNumber> &, gList<MixedSolution> &,
	  long &nevals, long &niters);

#endif   // NLIAP_H

