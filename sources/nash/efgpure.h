//
// FILE: efgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef EFGPURE_H
#define EFGPURE_H

#include "base/base.h"
#include "base/gstatus.h"
#include "game/efg.h"
#include "behavsol.h"
#include "subsolve.h"

class efgEnumPure : public SubgameSolver  {
private:
  int m_stopAfter;

  void SolveSubgame(const FullEfg &, const EFSupport &,
		    gList<BehavSolution> &, gStatus &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_ENUMPURE_EFG; }

public:
  efgEnumPure(int p_stopAfter);
  virtual ~efgEnumPure();
};

#endif    // EFGPURE_H


