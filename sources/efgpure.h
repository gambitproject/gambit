//
// FILE: efgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef EFGPURE_H
#define EFGPURE_H

#include "efg.h"
#include "behavsol.h"
#include "glist.h"

#include "subsolve.h"


int EnumPure(const EFSupport &, const gArray<gNumber> &,
	     gList<BehavSolution> &, double &time);

#include "subsolve.h"

class EfgPSNEBySubgame : public SubgameSolver  {
  private:
    gArray<gNumber> values;
   
    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_PURENASHSUB; }    

  public:
    EfgPSNEBySubgame(const EFSupport &, const gArray<gNumber> &, int max = 0);
    virtual ~EfgPSNEBySubgame();
};

#endif    // EFGPURE_H


