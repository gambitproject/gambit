//
// FILE: seqform.h -- Interface to Sequence Form solution module
//
// $Id$
//

#ifndef SEQFORM_H
#define SEQFORM_H

#include "glist.h"
#include "gstatus.h"
#include "behavsol.h"

class SeqFormParams     {
  public:
    int trace, stopAfter, maxdepth;
    Precision precision;
    gOutput *tracefile;
    gStatus &status;
    
    SeqFormParams(gStatus &status_ = gstatus);
};

int SeqForm(const EFSupport &, const SeqFormParams &,
	    gList<BehavSolution> &, int &npivots, double &time);

#endif    // SEQFORM_H



