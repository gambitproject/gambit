//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of abstract class to compute Nash equilibria on nfgs
//

#ifndef NFGALGORITHM_H
#define NFGALGORITHM_H

#include "game/nfg.h"
#include "mixedsol.h"

class nfgNashAlgorithm {
public:
  virtual ~nfgNashAlgorithm() { }
  virtual gList<MixedSolution> Solve(const NFSupport &, gStatus &) = 0;
};

#endif  // NFGALGORITHM_H
