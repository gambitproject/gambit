//
// FILE: lemkesub.h -- Solve efg by lemke on nfg
//
// $Id$
//

#ifndef LEMKESUB_H
#define LEMKESUB_H

#include "subsolve.h"
#include "lemke.h"

class efgLcpNfgSolve : public SubgameSolver  {
private:
  int npivots;
  LemkeParams params;

  int SolveSubgame(const Efg &, const EFSupport &, gList<BehavSolution> &);
  EfgAlgType AlgorithmID() const { return EfgAlg_LEMKESUB; }    

public:
  efgLcpNfgSolve(const EFSupport &, const LemkeParams &, int max = 0);
  virtual ~efgLcpNfgSolve();

  int NumPivots(void) const   { return npivots; }
};

#endif   // LEMKESUB_H
