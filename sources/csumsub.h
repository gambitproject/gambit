//
// FILE: csumsub.h -- Solve efg by LP on nfg
//
// $Id$
//

#ifndef CSUMSUB_H
#define CSUMSUB_H

#include "subsolve.h"
#include "nfgcsum.h"

class ZSumBySubgame : public SubgameSolver  {
  private:
    int npivots;
    ZSumParams params;
    gArray<gNumber> values;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_ZSUMSUB; }    

  public:
    ZSumBySubgame(const EFSupport &, const ZSumParams &,
		  const gArray<gNumber> &, int max = 0);
    virtual ~ZSumBySubgame();

    int NumPivots(void) const   { return npivots; }
};

int ZSum(const EFSupport &, const ZSumParams &, const gArray<gNumber> &,
	 gList<BehavSolution> &, int &npivots, double &time);



#endif    // CSUMSUB_H

