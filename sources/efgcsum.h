//
// FILE: efgcsum.h -- Interface to Constant Sum Extensive Form Game Solver 
//
// $Id$
//

#ifndef EFGCSUM_H
#define EFGCSUM_H

#include "glist.h"
#include "gstatus.h"
#include "tableau.h"
#include "lpsolve.h"
#include "behavsol.h"

class CSSeqFormParams     {
  public:
    int trace, stopAfter;
    gPrecision precision;
    gOutput *tracefile;
    gStatus &status;
    
    CSSeqFormParams(gStatus &status_ = gstatus);
};

int CSSeqForm(const EFSupport &, const CSSeqFormParams &,
	      gList<BehavSolution> &, int &npivots, double &time);


#include "subsolve.h"

class CSSeqFormBySubgame : public SubgameSolver  {
  private:
    long npivots;
    CSSeqFormParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_CSSEQFORM; }    

  public:
    CSSeqFormBySubgame(const EFSupport &,
		       const CSSeqFormParams &, int max = 0);
    virtual ~CSSeqFormBySubgame();

    long NumPivots(void) const  { return npivots; }
};


#endif    // EFGCSUM_H



