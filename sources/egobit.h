//
// FILE: egobit.h -- Interface to extensive form Gobit solution module
//
// $Id$
//

#ifndef EGOBIT_H
#define EGOBIT_H

#include "algutils.h"
#include "glist.h"

#include "efg.h"
#include "behavsol.h"

class EFQreParams : public FuncMinParams  {
public:
  int powLam;
  double minLam, maxLam, delLam;
  bool fullGraph;
  gOutput *pxifile;
  
  EFQreParams(gStatus & = gstatus);
  EFQreParams(gOutput &out, gOutput &pxi, gStatus & = gstatus);
};


void Qre(const Efg &, EFQreParams &,
	   const BehavProfile<gNumber> &, gList<BehavSolution > &,
	   long &nevals, long &nits);

void KQre(const Efg &E, EFQreParams &params,
	    const BehavProfile<gNumber> &start,
	    gList<BehavSolution> &solutions, 
	    long &nevals, long &nits);

#endif    // NGOBIT_H



