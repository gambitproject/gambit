//
// FILE: efgalleq.h -- enumerates all Nash equilibria, assuming genericity
//
// $Id$
//

#ifndef EFGALLEQ_H
#define EFGALLEQ_H

#include "base/base.h"
#include "efg.h"
#include "gstatus.h"
#include "behav.h"
#include "behavsol.h"
#include "epolenum.h" 
#include "efgensup.h"
#include "subsolve.h"

int AllEFNashSolve(const EFSupport &, const EfgPolEnumParams &, 
		   gList<BehavSolution> &, gStatus &,
		   long &nevals, double &time,
		   gList<const EFSupport> &singular_supports);


class efgPolEnumSolve : public SubgameSolver  {
private:
  int npivots;
  EfgPolEnumParams params;
  gArray<gNumber> values;

  void SolveSubgame(const FullEfg &, const EFSupport &,
		    gList<BehavSolution> &, gStatus &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_POLENUM_EFG; }    

public:
  efgPolEnumSolve(const EFSupport &, const EfgPolEnumParams &, int max = 0);
  virtual ~efgPolEnumSolve();
};

#endif    // EFGALLEQ_H




