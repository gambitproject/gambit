//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Algorithm to compute pure strategy equilibria on normal forms
//

#ifndef NFGPURE_H
#define NFGPURE_H

#include "base/base.h"
#include "base/gstatus.h"
#include "nfgalgorithm.h"

class nfgEnumPure : public nfgNashAlgorithm {
private:
  int m_stopAfter;

public:
  nfgEnumPure(void) : m_stopAfter(0) { }
  virtual ~nfgEnumPure() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  gText GetAlgorithm(void) const { return "EnumPure"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

#endif // NFGPURE_H


