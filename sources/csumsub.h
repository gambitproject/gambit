//
// FILE: csumsub.h -- Solve efg by LP on nfg
//
// $Id$
//

#ifndef CSUMSUB_H
#define CSUMSUB_H

#include "subsolve.h"
#include "nfgcsum.h"

class efgLpNfgSolve : public SubgameSolver  {
private:
  int npivots;
  ZSumParams params;

  void SolveSubgame(const FullEfg &, const EFSupport &, gList<BehavSolution> &);
  EfgAlgType AlgorithmID() const { return algorithmEfg_LP_NFG; }    

public:
  efgLpNfgSolve(const EFSupport &, const ZSumParams &, int max = 0);
  virtual ~efgLpNfgSolve();

  int NumPivots(void) const   { return npivots; }
};

#endif    // CSUMSUB_H

