//
// FILE: simpsub.h -- Solve efg by simpdiv on nfg
//
// $Id$
//

#ifndef SIMPSUB_H
#define SIMPSUB_H

#include "simpdiv.h"
#include "subsolve.h"

class SimpdivBySubgame : public SubgameSolver  {
  private:
    int nevals;
    SimpdivParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_SIMPDIVSUB; }    

  public:
    SimpdivBySubgame(const EFSupport &, const SimpdivParams &, int max = 0);
    virtual ~SimpdivBySubgame();

    int NumEvals(void) const    { return nevals; }
};

int Simpdiv(const EFSupport &, const SimpdivParams &,
	    gList<BehavSolution> &, int &nevals, int &niters, double &time);


#endif   // SIMPSUB_H

