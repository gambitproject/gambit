//
// FILE: nfgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef NFGPURE_H
#define NFGPURE_H

#include "base/base.h"
#include "game/nfg.h"
#include "game/mixedsol.h"
#include "gstatus.h"

class nfgEnumPure {
private:
  int m_stopAfter;

public:
  nfgEnumPure(void) : m_stopAfter(0) { }
  virtual ~nfgEnumPure() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  void Solve(const NFSupport &, gStatus &, gList<MixedSolution> &);
};

#endif // NFGPURE_H


