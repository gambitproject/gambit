//
// FILE: subnlemk.h -- Solve efg by lemke on nfg
//
// $Id$
//

#ifndef SUBNLEMK_H
#define SUBNLEMK_H

#include "lemke.h"
#include "subsolve.h"

template <class T> class LemkeBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    LemkeParams params;

    int SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_LEMKESUB; }    

  public:
    LemkeBySubgame(const Efg<T> &E, const LemkeParams &, int max = 0);
    virtual ~LemkeBySubgame();

    int NumPivots(void) const   { return npivots; }
};


#endif   // SUBNLEMK_H
