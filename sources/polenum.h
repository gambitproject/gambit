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
  gOutput *tracefile;
  gStatus &status;

  PolEnumParams(gStatus &status_=gstatus);
};


int PolEnum(const NFSupport &, const PolEnumParams &, 
	    gList<MixedSolution> &, long &nevals, double &time,
	    bool &is_singular);

#endif    // POLENUM_H




