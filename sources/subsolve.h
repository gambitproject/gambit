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

class NFSupport;
template <class T> class SubgameSolver   {
  private:
    int max_solns, subgame_number;
    double time;
    Efg efg;
    EFSupport support;
    BehavProfile<T> solution;
    gList<BehavSolution> solutions;

    gArray<gArray<Infoset *> *> infosets;

    void FindSubgames(Node *, gList<BehavSolution> &,
		      gList<EFOutcome *> &);

  protected:
    virtual int SolveSubgame(const Efg &, const EFSupport &,
			     gList<BehavSolution> &) = 0;
    // changed to return int = status.Get()

    virtual void ViewSubgame(int, const Efg &);

    virtual void ViewNormal(const Nfg &, NFSupport *&);
    virtual void SelectSolutions(int, const Efg &, gList<BehavSolution> &);
    virtual EfgAlgType AlgorithmID() const = 0;

  public:
    SubgameSolver(const Efg &E, const EFSupport &, int maxsol = 0);
    virtual ~SubgameSolver();
    
    void Solve(void);

    double Time(void) const   { return time; }
    const gList<BehavSolution> &GetSolutions(void) const
      { return solutions; }
};


#endif   // SUBSOLVE_H



