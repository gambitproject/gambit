//
// FILE: epolenum.h -- Interface to Andy's polynomial solver
//                     for extensive form games
//
// $Id$
//

#ifndef EPOLENUM_H
#define EPOLENUM_H

#include "base/base.h"
#include "game/efg.h"
#include "algutils.h"
#include "game/behav.h"
#include "game/behavsol.h"

class EfgPolEnumParams : public AlgParams {
public:
  EfgPolEnumParams(void);
};


int EfgPolEnum(const EFSupport &, const EfgPolEnumParams &, 
	       gList<BehavSolution> &, gStatus &, long &nevals, double &time,
	       bool &is_singular);

BehavSolution PolishEquilibrium(const EFSupport &, const BehavSolution &, 
				bool &is_singular);

#endif    // EPOLENUM_H




