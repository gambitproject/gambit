//
// FILE: lemkesub.h -- Solve efg by lemke on nfg
//
// $Id$
//

#ifndef LEMKESUB_H
#define LEMKESUB_H

#include "lemke.h"
#include "subsolve.h"

template <class T> class LemkeBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    LemkeParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_LEMKESUB; }    

  public:
    LemkeBySubgame(const Efg &E, const EFSupport &,
		   const LemkeParams &, int max = 0);
    virtual ~LemkeBySubgame();

    int NumPivots(void) const   { return npivots; }
};


#endif   // LEMKESUB_H
