//
// FILE: csumsub.h -- Solve efg by LP on nfg
//
// $Id$
//

#ifndef CSUMSUB_H
#define CSUMSUB_H

#include "subsolve.h"
#include "nfgcsum.h"

template <class T> class ZSumBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    ZSumParams params;

    int SolveSubgame(const Efg<T> &, const EFSupport &,
		     gList<BehavSolution<T> > &);
    EfgAlgType AlgorithmID() const { return EfgAlg_ZSUMSUB; }    

  public:
    ZSumBySubgame(const Efg<T> &E, const EFSupport &,
		  const ZSumParams &, int max = 0);
    virtual ~ZSumBySubgame();

    int NumPivots(void) const   { return npivots; }
};


#endif    // CSUMSUB_H

