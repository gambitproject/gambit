//
// FILE: eliap.h -- Interface to extensive form Liapunov solution module
//
// $Id$
//

#ifndef ELIAP_H
#define ELIAP_H

#include "gambitio.h"
#include "gstatus.h"
#include "glist.h"

#include "efg.h"
#include "behav.h"
#include "behavsol.h"

#include "subsolve.h"

class EFLiapParams  {
  public:
    int trace, nTries, stopAfter, maxits1, maxitsN;
    double tol1, tolN;
    gOutput *tracefile;
    gStatus &status;

    EFLiapParams(gStatus & = gstatus);
};


bool Liap(const Efg<double> &, EFLiapParams &,
	  const BehavProfile<double> &, gList<BehavSolution<double> > &,
	  long &nevals, long &niters);


class EFLiapBySubgame : public SubgameSolver<double>  {
  private:
    int nevals, subgame_number;
    gPVector<int> infoset_subgames;
    EFLiapParams params;
    BehavProfile<double> start;
    
    int SolveSubgame(const Efg<double> &, gList<BehavSolution<double> > &);
    EfgAlgType AlgorithmID() const { return EfgAlg_ELIAPSUB; }    

  public:
    EFLiapBySubgame(const Efg<double> &E, const EFLiapParams &,
		    const BehavProfile<double> &, int max = 0);
    virtual ~EFLiapBySubgame();

    int NumEvals(void) const   { return nevals; }
};



#endif    // ELIAP_H









