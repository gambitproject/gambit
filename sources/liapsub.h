//
// FILE: liapsub.h -- Solve efg by liap on nfg
//
// $Id$
//

#ifndef LIAPSUB_H
#define LIAPSUB_H

#include "nliap.h"
#include "subsolve.h"

class NFLiapBySubgame : public SubgameSolver<double>  {
  private:
    int nevals;
    NFLiapParams params;
    BehavProfile<double> start;

    int SolveSubgame(const Efg<double> &,
		      gList<BehavSolution<double> > &);
    int AlgorithmID() const { return id_NLIAPSUB; }    

  public:
    NFLiapBySubgame(const Efg<double> &E, const NFLiapParams &,
		    const BehavProfile<double> &, int max = 0);
    virtual ~NFLiapBySubgame();

    int NumEvals(void) const   { return nevals; }
};

#endif   // LIAPSUB_H
