//
// FILE: seqeq.h  -- Module to compute sequential equilibirum
//
// $Id$ 
//

#ifndef SEQEQ_H
#define SEQEQ_H

#include "efg.h"
#include "algutils.h"
#include "efstrat.h"
#include "efbasis.h"
#include "behavsol.h"

class SeqEquilibParams : public AlgParams {
public:
  SeqEquilibParams(void);
};


int SequentialEquilib(const EFBasis &, const EFSupport &, 
		      const SeqEquilibParams &, gList<BehavSolution> &, 
		      gStatus &, long &nevals, double &time);


#endif    // SEQEQ_H

