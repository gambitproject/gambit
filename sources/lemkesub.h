//
// FILE: lemkesub.h -- Solve efg by lemke on nfg
//
// $Id$
//

#ifndef LEMKESUB_H
#define LEMKESUB_H

#include "lemke.h"
#include "behavsol.h"

int Lemke(const EFSupport &, const LemkeParams &, 
	  gList<BehavSolution> &, int &npivots, double &time);

#endif   // LEMKESUB_H
