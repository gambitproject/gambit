//
// FILE: eliap.h -- Interface to extensive form Liapunov solution module
//
// $Id$
//

#ifndef ELIAP_H
#define ELIAP_H

#include "algutils.h"
#include "glist.h"

#include "efg.h"
#include "behavsol.h"

#include "subsolve.h"

class EFLiapParams : public FuncMinParams {
public:
  int nTries;
  
  EFLiapParams(gStatus & = gstatus);
};

class efgLiapSolve : public SubgameSolver  {
private:
  int nevals, subgame_number;
  gPVector<int> infoset_subgames;
  EFLiapParams params;
  BehavProfile<gNumber> start;
    
  void SolveSubgame(const Efg &, const EFSupport &, gList<BehavSolution> &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_LIAP_EFG; }    

public:
  efgLiapSolve(const Efg &, const EFLiapParams &,
	       const BehavProfile<gNumber> &, int max = 0);
  virtual ~efgLiapSolve();
  
  int NumEvals(void) const   { return nevals; }
};

#endif    // ELIAP_H









