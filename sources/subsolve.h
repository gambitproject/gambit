//
// FILE: subsolve.h -- Interface to generic subgame solver
//
// $Id$
//

#ifndef SUBSOLVE_H
#define SUBSOLVE_H

#include "efg.h"
#include "nfg.h"
#include "behavsol.h"

class efgAlgorithm {
public:
  virtual ~efgAlgorithm();

  virtual gList<BehavSolution> Solve(const EFSupport &) = 0;
};

class NFSupport;
class SubgameSolver   {
private:
  int max_solns, subgame_number;
  double time;
  BehavProfile<gNumber> *solution;
  gList<BehavSolution> solutions;

  gArray<gArray<Infoset *> *> infosets;

  void FindSubgames(const EFSupport &, Node *, gList<BehavSolution> &,
		    gList<EFOutcome *> &);
  
protected:
  virtual void SolveSubgame(const Efg &, const EFSupport &,
			    gList<BehavSolution> &) = 0;
  virtual void ViewSubgame(int, const Efg &);

  virtual void ViewNormal(const Nfg &, NFSupport *&);
  virtual void SelectSolutions(int, const Efg &, gList<BehavSolution> &);
  virtual EfgAlgType AlgorithmID() const = 0;

public:
  SubgameSolver(int maxsol = 0);
  virtual ~SubgameSolver();
    
  gList<BehavSolution> Solve(const EFSupport &);
  
  double Time(void) const   { return time; }
};


#endif   // SUBSOLVE_H



