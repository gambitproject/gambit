//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface of algorithm to compute behavior strategy equilibria
// of constant sum extensive form games via linear programming
//

#ifndef EFGCSUM_H
#define EFGCSUM_H

#include "numerical/tableau.h"
#include "numerical/lpsolve.h"
#include "efgalgorithm.h"

template <class T> class efgLp : public efgNashAlgorithm {
private:
  T maxpay, minpay;
  int ns1,ns2,ni1,ni2;
  gList<BFS<T> > List;
  gList<Infoset *> isets1, isets2;

  void FillTableau(const EFSupport &,
		   gMatrix<T> &, const Node *, T ,int ,int , int ,int );
  void GetSolutions(const EFSupport &, gList<BehavSolution> &) const;
  int Add_BFS(/*const*/ LPSolve<T> &B);
  
  void GetProfile(const EFSupport &, gDPVector<T> &v, const BFS<T> &sol,
		  const Node *n, int s1,int s2) const;

public:
  efgLp(void);
  virtual ~efgLp() { }

  gText GetAlgorithm(void) const { return "Lp"; }
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);
};


#endif    // EFGCSUM_H



