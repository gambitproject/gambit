//
// FILE: psnesub.h -- Solve efg by subgames in pure nfg strategies
//
// $Id$
//

#ifndef PSNESUB_H
#define PSNESUB_H

#include "gstatus.h"
#include "subsolve.h"

class PureNashBySubgame : public SubgameSolver  {
private:
  gStatus &m_status;

  int SolveSubgame(const Efg &, const EFSupport &,
		   gList<BehavSolution> &);
  EfgAlgType AlgorithmID() const { return EfgAlg_PURENASHSUB; }    
  
public:
  PureNashBySubgame(const EFSupport &, gStatus &, int max = 0);
  virtual ~PureNashBySubgame();
};

int EnumPureNfg(const EFSupport &, int, gStatus &,
		gList<BehavSolution> &, double &time);
	     
#endif   // PSNESUB_H

