//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerates all Nash equilibria in a normal form game, via solving
// systems of polynomial equations
//

#ifndef NFGALLEQ_H
#define NFGALLEQ_H

#include "nfgalgorithm.h"

class nfgPolEnum : public nfgNashAlgorithm {
private:
  int m_stopAfter;

public:
  nfgPolEnum(void);
  virtual ~nfgPolEnum() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  gText GetAlgorithm(void) const { return "PolEnum"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

#endif    // NFGALLEQ_H




