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

  void SolveSubgame(const Efg &, const EFSupport &, gList<BehavSolution> &);
  EfgAlgType AlgorithmID() const { return EfgAlg_ZSUMSUB; }    

public:
  efgLpNfgSolve(const EFSupport &, const ZSumParams &, int max = 0);
  virtual ~efgLpNfgSolve();

  int NumPivots(void) const   { return npivots; }
};

#endif    // CSUMSUB_H

