//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerates all Nash equilibria in an extensive form game, via solving
// systems of polynomial equations
//

#ifndef EFGALLEQ_H
#define EFGALLEQ_H

#include "efgalgorithm.h"

class efgPolEnum : public efgNashAlgorithm {
private:
  int m_stopAfter;

public:
  efgPolEnum(void);
  virtual ~efgPolEnum() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  gText GetAlgorithm(void) const { return "PolEnum"; }
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);
};
  


#endif    // EFGALLEQ_H




