//
// FILE: psnesub.h -- Solve efg by subgames in pure nfg strategies
//
// $Id$
//

#ifndef PSNESUB_H
#define PSNESUB_H

#include "subsolve.h"

class PureNashBySubgame : public SubgameSolver  {
  private:
    gArray<gNumber> values;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_PURENASHSUB; }    

  public:
    PureNashBySubgame(const EFSupport &, const gArray<gNumber> &, int max = 0);
    virtual ~PureNashBySubgame();
};

int EnumPureNfg(const EFSupport &, const gArray<gNumber> &values,
		gList<BehavSolution> &, double &time);
	     
#endif   // PSNESUB_H

