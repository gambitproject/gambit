//
// FILE: enumsub.h -- Solve efg by enumeration in nfg
//
// $Id$
//

#ifndef ENUMSUB_H
#define ENUMSUB_H

#include "subsolve.h"
#include "enum.h"

class EnumBySubgame : public SubgameSolver  {
  private:
    int npivots;
    EnumParams params;
    gArray<gNumber> values;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_ENUMSUB; }    

  public:
    EnumBySubgame(const EFSupport &, const gArray<gNumber> &values,
		  const EnumParams &, int max = 0);
    virtual ~EnumBySubgame();

    int NumPivots(void) const   { return npivots; }
};

int Enum(const EFSupport &, const EnumParams &, const gArray<gNumber> &,
	 gList<BehavSolution> &, long &npivots, double &time);

#endif   // ENUMSUB_H


