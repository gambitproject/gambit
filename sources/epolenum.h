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
#include "gstatus.h"
#include "behav.h"
#include "behavsol.h"

class EfgPolEnumParams     {
public:
  int trace, stopAfter;
  Precision precision;
  gOutput *tracefile;
  gStatus &status;

  EfgPolEnumParams(gStatus &status_=gstatus);
};


int EfgPolEnum(const EFSupport &, const EfgPolEnumParams &, 
	       gList<BehavSolution> &, long &nevals, double &time);

#endif    // EPOLENUM_H




