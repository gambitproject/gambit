//
// FILE: simpsub.h -- Solve efg by simpdiv on nfg
//
// $Id$
//

#ifndef SIMPSUB_H
#define SIMPSUB_H

#include "simpdiv.h"
#include "subsolve.h"

class efgSimpDivNfgSolve : public SubgameSolver  {
private:
  int nevals;
  SimpdivParams params;

  int SolveSubgame(const Efg &, const EFSupport &, gList<BehavSolution> &);
  EfgAlgType AlgorithmID() const { return EfgAlg_SIMPDIVSUB; }    

public:
  efgSimpDivNfgSolve(const EFSupport &, const SimpdivParams &, int max = 0);
  virtual ~efgSimpDivNfgSolve();

  int NumEvals(void) const    { return nevals; }
};

#endif   // SIMPSUB_H

