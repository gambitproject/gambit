//
// FILE: lemkesub.h -- Solve efg by lemke on nfg
//
// $Id$
//

#ifndef LEMKESUB_H
#define LEMKESUB_H

#include "subsolve.h"
#include "lemke.h"

class LemkeBySubgame : public SubgameSolver  {
  private:
    int npivots;
    LemkeParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_LEMKESUB; }    

  public:
    LemkeBySubgame(const EFSupport &, const LemkeParams &, int max = 0);
    virtual ~LemkeBySubgame();

    int NumPivots(void) const   { return npivots; }
};


int Lemke(const EFSupport &, const LemkeParams &,
	  gList<BehavSolution> &, int &npivots, double &time);

#endif   // LEMKESUB_H
