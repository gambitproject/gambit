//
// FILE: efgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef EFGPURE_H
#define EFGPURE_H

#include "glist.h"
#include "gstatus.h"
#include "efg.h"
#include "behavsol.h"
#include "subsolve.h"

class efgEnumPure : public SubgameSolver  {
private:
  int m_stopAfter;
  gStatus &m_status;

  void SolveSubgame(const FullEfg &, const EFSupport &,
		    gList<BehavSolution> &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_ENUMPURE_EFG; }

public:
  efgEnumPure(int p_stopAfter, gStatus &p_status);
  virtual ~efgEnumPure();
};

#endif    // EFGPURE_H


