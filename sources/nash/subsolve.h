//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria of an extensive form game by recursively
// solving subgames
//

#ifndef SUBSOLVE_H
#define SUBSOLVE_H

#include "efgalgorithm.h"
#include "nfgalgorithm.h"

class SubgameSolver : public efgNashAlgorithm  {
private:
  bool m_isPerfectRecall;
  double time;
  BehavProfile<gNumber> *solution;
  gList<BehavSolution> solutions;
  efgNashAlgorithm *m_efgAlgorithm;
  nfgNashAlgorithm *m_nfgAlgorithm;

  gArray<gArray<Infoset *> *> infosets;

  void FindSubgames(const EFSupport &, gStatus &, 
		    Node *, gList<BehavSolution> &, gList<efgOutcome *> &);
  
public:
  SubgameSolver(void) : m_efgAlgorithm(0), m_nfgAlgorithm(0) { }
  virtual ~SubgameSolver();
    
  gText GetAlgorithm(void) const;
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);

  void SetAlgorithm(efgNashAlgorithm *p_algorithm)
    { m_efgAlgorithm = p_algorithm; m_nfgAlgorithm = 0; }
  void SetAlgorithm(nfgNashAlgorithm *p_algorithm)
    { m_nfgAlgorithm = p_algorithm; m_efgAlgorithm = 0; }
  
  double Time(void) const   { return time; }
};


#endif   // SUBSOLVE_H



