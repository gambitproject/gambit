//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of abstract Nash equilibrium algorithm class
//

#ifndef EFGALGORITHM_H
#define EFGALGORITHM_H

#include "game/efg.h"
#include "behavsol.h"

class efgNashAlgorithm {
public:
  virtual ~efgNashAlgorithm() { }
  virtual gList<BehavSolution> Solve(const EFSupport &, gStatus &) = 0;
};

#endif  // EFGALGORITHM_H
