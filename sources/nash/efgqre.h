//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to (agent) QRE computation for extensive forms
//

#ifndef EFGQRE_H
#define EFGQRE_H

#include "base/base.h"

#include "game/efg.h"
#include "behavsol.h"

//
// Computes a branch of the agent logistic quantal response equilibrium 
// correspondence.
//
// Currently, only starting from the centroid at lambda = 0 is supported.
// Eventually, starting from a Nash equilibrium at lambda = infinity will
// be added.
//

class QreEfg {
private:
  double m_maxLam, m_stepSize;
  bool m_fullGraph;

public:
  // LIFECYCLE
  QreEfg(void);

  // ACCESSING AND SETTING ALGORITHM PARAMETERS
  double GetMaxLambda(void) const { return m_maxLam; }
  void SetMaxLambda(double p_maxLam) { m_maxLam = p_maxLam; }

  double GetStepSize(void) const { return m_stepSize; }
  void SetStepSize(double p_stepSize) { m_stepSize = p_stepSize; }

  bool GetFullGraph(void) const { return m_fullGraph; }
  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }

  // RUN THE ALGORITHM
  void Solve(const Efg::Game &p_efg, gOutput &p_pxifile,
	     gStatus &p_status, gList<BehavSolution> &p_solutions);
};

#ifdef WITH_KQRE
void KQre(const Efg::Game &E, EFQreParams &params, gOutput &,
	  const BehavProfile<gNumber> &start, gList<BehavSolution> &solutions, 
	  gStatus &, long &nevals, long &nits);
#endif  // WITH_KQRE

#endif   // EFGQRE_H




