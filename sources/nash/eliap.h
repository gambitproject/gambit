//
// FILE: eliap.h -- Interface to extensive form Liapunov solution module
//
// $Id$
//

#ifndef ELIAP_H
#define ELIAP_H

#include "base/base.h"
#include "algutils.h"

#include "game/efg.h"
#include "behavsol.h"

#include "subsolve.h"

class EFLiapParams : public FuncMinParams {
public:
  int nTries;
  
  EFLiapParams(void);
};

class efgLiapSolve : public SubgameSolver  {
private:
  int nevals, subgame_number;
  gPVector<int> infoset_subgames;
  EFLiapParams params;
  BehavProfile<gNumber> start;
    
  void SolveSubgame(const FullEfg &, const EFSupport &,
		    gList<BehavSolution> &, gStatus &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_LIAP_EFG; }    

public:
  efgLiapSolve(const Efg::Game &, const EFLiapParams &,
	       const BehavProfile<gNumber> &, int max = 0);
  virtual ~efgLiapSolve();
  
  int NumEvals(void) const   { return nevals; }
};

#endif    // ELIAP_H









