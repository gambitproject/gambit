//
// FILE: psnesub.h -- Solve efg by subgames in pure mixed strategies
//
// $Id$
//

#ifndef PSNESUB_H
#define PSNESUB_H

#include "gstatus.h"
#include "subsolve.h"

class efgEnumPureNfgSolve : public SubgameSolver  {
private:
  int m_stopAfter;
  gStatus &m_status;

  void SolveSubgame(const Efg &, const EFSupport &, gList<BehavSolution> &);
  EfgAlgType AlgorithmID() const { return EfgAlg_PURENASHSUB; }    
  
public:
  efgEnumPureNfgSolve(const EFSupport &, int p_stopAfter, gStatus &);
  virtual ~efgEnumPureNfgSolve();
};

#endif   // PSNESUB_H

