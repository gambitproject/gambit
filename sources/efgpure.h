//
// FILE: efgpure.h -- Interface to pure-strategy Nash equilibrium finder
//
// $Id$
//

#ifndef EFGPURE_H
#define EFGPURE_H

#include "efg.h"
#include "behavsol.h"
#include "glist.h"

#include "subsolve.h"


int FindPureNash(const Efg &, const EFSupport &, gList<BehavSolution> &);


template <class T> class EfgPSNEBySubgame : public SubgameSolver<T>  {
  private:
    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_PURENASHSUB; }    

  public:
    EfgPSNEBySubgame(const Efg &E, const EFSupport &, int max = 0);
    virtual ~EfgPSNEBySubgame();
};


#endif    // EFGPURE_H


