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

class efgEnumPure : public SubgameSolver  {
private:
  void SolveSubgame(const Efg &, const EFSupport &,
		    gList<BehavSolution> &);
  EfgAlgType AlgorithmID() const { return EfgAlg_PURENASHSUB; }    

public:
  efgEnumPure(int max = 0);
  virtual ~efgEnumPure();
};

#endif    // EFGPURE_H


