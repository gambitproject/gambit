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


template <class T> int FindPureNash(const Efg<T> &N,
				    gList<BehavSolution<T> > &);


template <class T> class EfgPSNEBySubgame : public SubgameSolver<T>  {
  private:
    int SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    EfgAlgType AlgorithmID() const { return EfgAlg_PURENASHSUB; }    

  public:
    EfgPSNEBySubgame(const Efg<T> &E, int max = 0);
    virtual ~EfgPSNEBySubgame();
};


#endif    // EFGPURE_H


