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
  bool m_isPerfectRecall;
  int max_solns, subgame_number;
  double time;
  BehavProfile<gNumber> *solution;
  gList<BehavSolution> solutions;

  gArray<gArray<Infoset *> *> infosets;

  void FindSubgames(const EFSupport &, gStatus &, 
		    Node *, gList<BehavSolution> &, gList<Efg::Outcome> &);
  
protected:
  virtual void SolveSubgame(const FullEfg &, const EFSupport &,
			    gList<BehavSolution> &, gStatus &) = 0;
  virtual void ViewSubgame(int, const FullEfg &, EFSupport &);

  virtual void ViewNormal(const Nfg &, NFSupport &);
  virtual void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);
  virtual EfgAlgType AlgorithmID() const = 0;

public:
  SubgameSolver(int maxsol = 0);
  virtual ~SubgameSolver();
    
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);
  
  double Time(void) const   { return time; }
};


#endif   // SUBSOLVE_H



