//
// FILE: polensub.h -- Solve efg by polynomial enumeration in nfg
//
// $Id$
//

#ifndef POLENSUB_H
#define POLENSUB_H

#include "polenum.h"
#include "behavsol.h"
#include "subsolve.h"

class PolEnumBySubgame : public SubgameSolver  {
  private:
    int nevals;
    PolEnumParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_POLENSUB; }    

  public:
    PolEnumBySubgame(const EFSupport &, const PolEnumParams &, int max = 0);
    virtual ~PolEnumBySubgame();

    int NumEvals(void) const   { return nevals; }
};

int PolEnum(const EFSupport &, const PolEnumParams &,
	    gList<BehavSolution> &, long &nevals, double &time);

#endif   // POLENSUB_H


