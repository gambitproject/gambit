//
// FILE: efgcsum.h -- Interface to Constant Sum Extensive Form Game Solver 
//
// $Id$
//

#ifndef EFGCSUM_H
#define EFGCSUM_H

#include "glist.h"
#include "algutils.h"
#include "tableau.h"
#include "lpsolve.h"
#include "behavsol.h"

class CSSeqFormParams : public AlgParams {
  public:
    CSSeqFormParams(gStatus &status_ = gstatus);
};

#include "subsolve.h"

class efgLpSolve : public SubgameSolver  {
private:
  long npivots;
  CSSeqFormParams params;

  void SolveSubgame(const Efg &, const EFSupport &,
		    gList<BehavSolution> &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_LP_EFG; }    

public:
  efgLpSolve(const EFSupport &, const CSSeqFormParams &, int max = 0);
  virtual ~efgLpSolve();
  
  long NumPivots(void) const  { return npivots; }
};


#endif    // EFGCSUM_H



