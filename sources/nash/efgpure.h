//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute pure-strategy Nash equilibria on extensive form games
//

#ifndef EFGPURE_H
#define EFGPURE_H

#include "base/base.h"
#include "base/gstatus.h"
#include "efgalgorithm.h"

class efgEnumPure : public efgNashAlgorithm {
private:
  int m_stopAfter;

public:
  efgEnumPure(void) : m_stopAfter(0) { }
  virtual ~efgEnumPure() { }

  gText GetAlgorithm(void) const { return "EnumPure[EFG]"; }
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);
};

#endif    // EFGPURE_H


