//
// FILE: enumsub.h -- Solve efg by enumeration in nfg
//
// $Id$
//

#ifndef ENUMSUB_H
#define ENUMSUB_H

#include "subsolve.h"
#include "enum.h"

template <class T> class EnumBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    EnumParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution<T> > &);
    EfgAlgType AlgorithmID() const { return EfgAlg_ENUMSUB; }    

  public:
    EnumBySubgame(const Efg &E, const EFSupport &,
		  const EnumParams &, int max = 0);
    virtual ~EnumBySubgame();

    int NumPivots(void) const   { return npivots; }
};


#endif   // ENUMSUB_H


