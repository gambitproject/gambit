//
// FILE: epolenum.h -- Interface to Andy's polynomial solver
//                     for extensive form games
//
// $Id$
//

#ifndef EPOLENUM_H
#define EPOLENUM_H

#include "efg.h"
#include "glist.h"
#include "algutils.h"
#include "behav.h"
#include "behavsol.h"

class EfgPolEnumParams : public AlgParams {
public:
  EfgPolEnumParams(gStatus &status_=gstatus);
};


int EfgPolEnum(const EFSupport &, const EfgPolEnumParams &, 
	       gList<BehavSolution> &, long &nevals, double &time,
	       bool &is_singular);

#endif    // EPOLENUM_H




