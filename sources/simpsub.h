//
// FILE: simpsub.h -- Solve efg by simpdiv on nfg
//
// $Id$
//

#ifndef SIMPSUB_H
#define SIMPSUB_H

#include "simpdiv.h"
#include "subsolve.h"

template <class T> class SimpdivBySubgame : public SubgameSolver<T>  {
  private:
    int nevals;
    SimpdivParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_SIMPDIVSUB; }    

  public:
    SimpdivBySubgame(const Efg &E, const EFSupport &,
		     const SimpdivParams &, int max = 0);
    virtual ~SimpdivBySubgame();

    int NumEvals(void) const    { return nevals; }
};


#endif   // SIMPSUB_H

