//
// FILE: egobit.h -- Interface to extensive form Gobit solution module
//
// $Id$
//

#ifndef EGOBIT_H
#define EGOBIT_H

#include "base/base.h"
#include "algutils.h"

#include "efg.h"
#include "behavsol.h"

class EFQreParams : public FuncMinParams  {
public:
  int powLam;
  double minLam, maxLam, delLam;
  bool fullGraph;
  gOutput *pxifile;
  
  EFQreParams(void);
  EFQreParams(gOutput &out, gOutput &pxi);
};


void Qre(const Efg &, EFQreParams &,
	 const BehavProfile<gNumber> &, gList<BehavSolution > &,
	 gStatus &, long &nevals, long &nits);

void KQre(const Efg &E, EFQreParams &params,
	  const BehavProfile<gNumber> &start, gList<BehavSolution> &solutions, 
	  gStatus &, long &nevals, long &nits);

#endif    // NGOBIT_H



