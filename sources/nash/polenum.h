//
// FILE: polenum.h -- Interface to Andy's polynomial solver
//
// $Id$
//

#ifndef POLENUM_H
#define POLENUM_H

#include "base/base.h"
#include "game/nfg.h"
#include "algutils.h"
#include "game/mixed.h"
#include "mixedsol.h"

class PolEnumParams : public AlgParams  {
public:
  PolEnumParams(void);
};


int PolEnum(const NFSupport &, const PolEnumParams &, 
	    gList<MixedSolution> &, gStatus &,
	    long &nevals, double &time, bool &is_singular);

MixedSolution PolishEquilibrium(const NFSupport &, const MixedSolution &, 
				bool &is_singular);

#endif    // POLENUM_H




