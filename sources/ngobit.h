//
// FILE: ngobit.h -- Interface to normal form Gobit solution module
//
// $Id$
//

#ifndef NGOBIT_H
#define NGOBIT_H

#include "algutils.h"
#include "glist.h"

#include "nfg.h"
#include "mixedsol.h"

class NFQreParams : public FuncMinParams {
public:
  int powLam;
  double minLam, maxLam, delLam;
  bool fullGraph;
  gOutput *pxifile;
  
  NFQreParams(void);
  NFQreParams(gOutput &out, gOutput &pxi);
};


void Qre(const Nfg &, NFQreParams &,
	 const MixedProfile<gNumber> &, gList<MixedSolution> &, gStatus &,
	 long &nevals, long &nits);


void KQre(const Nfg &N, NFQreParams &params,
	    const MixedProfile<gNumber> &start,
	    gList<MixedSolution> &solutions, gStatus &,
	    long &nevals, long &nits);

#endif    // NGOBIT_H



