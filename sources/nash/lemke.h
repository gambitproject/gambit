//
// FILE: lemke.h -- Interface to Lemke solution module
//
// $Id$
//

#ifndef LEMKE_H
#define LEMKE_H

#include "base/base.h"
#include "game/nfg.h"
#include "mixedsol.h"
#include "algutils.h"

class LemkeParams : public AlgParams    {
public:
  int dup_strat, maxdepth;
  
  LemkeParams(void);
};

int Lemke(const NFSupport &, const LemkeParams &,
          gList<MixedSolution> &, gStatus &, int &npivots, double &time);
 

#endif    // LEMKE_H





