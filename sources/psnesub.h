//
// FILE: subnpsne.h -- Solve efg by subgames in pure nfg strategies
//
// $Id$
//

#ifndef SUBNPSNE_H
#define SUBNPSNE_H

#include "subsolve.h"
#include "nfgpure.h"

template <class T> class PureNashBySubgame : public SubgameSolver<T>  {
  private:
    int SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_PURENASHSUB; }    

  public:
    PureNashBySubgame(const Efg<T> &E, int max = 0);
    virtual ~PureNashBySubgame();
};

#endif   // SUBNPSNE_H
