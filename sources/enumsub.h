//
// FILE: subnenum.h -- Solve efg by enumeration in nfg
//
// $Id$
//

#ifndef SUBNENUM_H
#define SUBNENUM_H

#include "subsolve.h"
#include "enum.h"

template <class T> class EnumBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    EnumParams params;

    int SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_ENUMSUB; }    

  public:
    EnumBySubgame(const Efg<T> &E, const EnumParams &, int max = 0);
    virtual ~EnumBySubgame();

    int NumPivots(void) const   { return npivots; }
};


#endif   // SUBNENUM_H


