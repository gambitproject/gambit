//
// FILE: liapsub.h -- Solve efg by liap on nfg
//
// $Id$
//

#ifndef LIAPSUB_H
#define LIAPSUB_H

#include "nliap.h"
#include "subsolve.h"

class efgLiapNfgSolve : public SubgameSolver  {
private:
  int nevals, subgame_number;
  gPVector<int> infoset_subgames;
  NFLiapParams params;
  BehavProfile<gNumber> start;

  void SolveSubgame(const FullEfg &, const EFSupport &,
		    gList<BehavSolution> &, gStatus &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_LIAP_NFG; }    

public:
  efgLiapNfgSolve(const Efg &, const NFLiapParams &,
		  const BehavProfile<gNumber> &, int max = 0);
  virtual ~efgLiapNfgSolve();
  
  int NumEvals(void) const   { return nevals; }
};

#endif   // LIAPSUB_H
