//
// FILE: lemke.h -- Interface to Lemke solution module
//
// $Id$
//

#ifndef LEMKE_H
#define LEMKE_H

#include "nfg.h"
#include "mixedsol.h"
#include "glist.h"
#include "gstatus.h"

class LemkeParams     {
  public:
    int dup_strat, trace, stopAfter;
    Precision precision;
    gOutput *tracefile;
    gStatus &status;
    
    LemkeParams(gStatus & = gstatus);
};

int Lemke(const NFSupport &, const LemkeParams &,
          gList<MixedSolution> &, int &npivots, double &time);
 

#endif    // LEMKE_H





