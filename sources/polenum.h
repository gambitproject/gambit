//
// FILE: polenum.h -- Interface to Andy's polynomial solver
//
// $Id$
//

#ifndef POLENUM_H
#define POLENUM_H

#include "nfg.h"
#include "glist.h"
#include "algutils.h"
#include "mixed.h"
#include "mixedsol.h"

class PolEnumParams : public AlgParams  {
public:
  PolEnumParams(void);
};


int PolEnum(const NFSupport &, const PolEnumParams &, 
	    gList<MixedSolution> &, gStatus &,
	    long &nevals, double &time, bool &is_singular);

#endif    // POLENUM_H




