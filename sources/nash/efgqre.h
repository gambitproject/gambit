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

#include "efgalgorithm.h"

//
// Computes a branch of the agent logistic quantal response equilibrium 
// correspondence.
//
// Currently, only starting from the centroid at lambda = 0 is supported.
// Eventually, starting from a Nash equilibrium at lambda = infinity will
// be added.
//

class efgQre : public efgNashAlgorithm {
private:
  double m_maxLam, m_stepSize;
  bool m_fullGraph;

public:
  efgQre(void);
  virtual ~efgQre() { }

  double GetMaxLambda(void) const { return m_maxLam; }
  void SetMaxLambda(double p_maxLam) { m_maxLam = p_maxLam; }

  double GetStepSize(void) const { return m_stepSize; }
  void SetStepSize(double p_stepSize) { m_stepSize = p_stepSize; }

  bool GetFullGraph(void) const { return m_fullGraph; }
  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }

  gText GetAlgorithm(void) const { return "Qre[EFG]"; }
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);
};

#endif   // EFGQRE_H




