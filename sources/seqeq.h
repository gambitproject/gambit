//
// FILE: seqeq.h  -- Module to compute sequential equilibirum
//
// $Id$ 
//

#ifndef SEQEQ_H
#define SEQEQ_H

#include "efg.h"
#include "gstatus.h"
#include "efstrat.h"
#include "efbasis.h"
#include "behavsol.h"

class SeqEquilibParams     {
public:
  int trace, stopAfter;
  gPrecision precision;
  gOutput *tracefile;
  gStatus &status;

  SeqEquilibParams(gStatus &status_=gstatus);
};


int SequentialEquilib(const EFBasis &, const EFSupport &, 
		      const SeqEquilibParams &, gList<BehavSolution> &, 
		      long &nevals, double &time);


#endif    // SEQEQ_H

