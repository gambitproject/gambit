//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface of algorithm to compute mixed strategy equilibria
// of constant sum normal form games via linear programming
//

#ifndef NFGCSUM_H
#define NFGCSUM_H

#include "nfgalgorithm.h"
#include "numerical/lpsolve.h"

template <class T> class nfgLp : public nfgNashAlgorithm {
private:
  int Add_BFS(const NFSupport &, /*const*/ LPSolve<T> &B,
	      gList<BFS<T> > &);
  void GetSolutions(const NFSupport &, const gList<BFS<T> > &,
		    gList<MixedSolution > &,
		    const T &) const;

public:
  nfgLp(void);
  virtual ~nfgLp() { }

  gText GetAlgorithm(void) const { return "Lp"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

#endif    // NFGCSUM_H



