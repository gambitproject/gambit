//
// FILE: subncsum.h -- Solve efg by LP on nfg
//
// $Id$
//

#ifndef SUBNCSUM_H
#define SUBNCSUM_H

#include "subsolve.h"
#include "csum.h"

template <class T> class ZSumBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    ZSumParams params;

    int SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_ZSUMSUB; }    

  public:
    ZSumBySubgame(const Efg<T> &E, const ZSumParams &, int max = 0);
    virtual ~ZSumBySubgame();

    int NumPivots(void) const   { return npivots; }
};


#endif    // SUBNCSUM_H

