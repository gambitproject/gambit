//
// FILE: eliap.h -- Interface to extensive form Liapunov solution module
//
// $Id$
//

#ifndef ELIAP_H
#define ELIAP_H

#include "gambitio.h"
#include "gstatus.h"
#include "glist.h"

#include "efg.h"
#include "behav.h"
#include "behavsol.h"

class EFLiapParams  {
  public:
    int trace, maxits1, maxitsN;
    double tol1, tolN;
    gOutput *tracefile;
    gStatus &status;

    EFLiapParams(gStatus & = gstatus);
};


bool Liap(const Efg<double> &, EFLiapParams &,
	  const BehavProfile<double> &, gList<BehavSolution<double> > &,
	  long &nevals, long &niters);

#endif    // ELIAP_H









