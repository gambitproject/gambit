//
// FILE: seqform.h -- Interface to Sequence Form solution module
//
// $Id$
//

#ifndef SEQFORM_H
#define SEQFORM_H

#include "base/base.h"
#include "nash/algutils.h"
#include "nash/subsolve.h"
#include "game/behavsol.h"

class SeqFormParams : public AlgParams {
public:
  int maxdepth;
  
  SeqFormParams(void);
};

class efgLcpSolve : public SubgameSolver  {
private:
  int npivots;
  SeqFormParams params;
  gArray<gNumber> values;

  void SolveSubgame(const FullEfg &, const EFSupport &,
		    gList<BehavSolution> &, gStatus &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_LCP_EFG; }    

public:
  efgLcpSolve(const SeqFormParams &, int max = 0);
  virtual ~efgLcpSolve();
  
  int NumPivots(void) const  { return npivots; }
};

#endif    // SEQFORM_H





