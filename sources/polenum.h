//
// FILE: polenum.h -- Interface to Andy's polynomial solver
//
// $Id$
//

#ifndef POLENUM_H
#define POLENUM_H

#include "nfg.h"
#include "glist.h"
#include "gstatus.h"
#include "mixed.h"
#include "mixedsol.h"

class PolEnumParams     {
public:
  int trace, stopAfter;
  Precision precision;
  gOutput *tracefile;
  gStatus &status;

  PolEnumParams(gStatus &status_=gstatus);
};


int PolEnum(const NFSupport &, const PolEnumParams &, const gArray<gNumber> &,
	 gList<MixedSolution> &, long &nevals, double &time);

#endif    // POLENUM_H




