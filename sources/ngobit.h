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

class NFQreParams : public AlgParams {
  public:
    int powLam, maxits1, maxitsN;
    double minLam, maxLam, delLam, tol1, tolN;
    bool fullGraph;
    gOutput *pxifile;

    NFQreParams();
    NFQreParams(gOutput &out, gOutput &pxi);
};


void Qre(const Nfg &, NFQreParams &,
	   const MixedProfile<gNumber> &, gList<MixedSolution> &,
	   long &nevals, long &nits);


void KQre(const Nfg &N, NFQreParams &params,
	    const MixedProfile<gNumber> &start,
	    gList<MixedSolution> &solutions, 
	    long &nevals, long &nits);

#endif    // NGOBIT_H



