//
// FILE: seqeq.h  -- Module to compute sequential equilibirum
//
// $Id$ 
//

#ifndef SEQEQ_H
#define SEQEQ_H

#include "game/efg.h"
#include "algutils.h"
#include "game/efstrat.h"
#include "game/efbasis.h"
#include "game/behavsol.h"

class SeqEquilibParams : public AlgParams {
public:
  SeqEquilibParams(void);
};


int SequentialEquilib(const EFBasis &, const EFSupport &, 
		      const SeqEquilibParams &, gList<BehavSolution> &, 
		      gStatus &, long &nevals, double &time);


#endif    // SEQEQ_H

