//#
//# FILE: subsolve.h -- Interface to generic subgame solver
//#
//# $Id$
//#

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
    Efg<T> efg;
    BehavProfile<T> solution;
    gList<BehavSolution<T> > solutions;

    gArray<gArray<Infoset *> *> infosets;

    void FindSubgames(Node *, gList<BehavSolution<T> > &,
		      gList<Outcome *> &);

  protected:
    virtual int SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &) = 0;
    // changed to return int = status.Get()

    virtual void ViewSubgame(int, const Efg<T> &);

    virtual void ViewNormal(const Nfg<T> &, NFSupport *&);
    virtual void SelectSolutions(int, const Efg<T> &, gList<BehavSolution<T> > &);
    virtual int AlgorithmID() const = 0;

  public:
    SubgameSolver(const Efg<T> &E, int maxsol = 0);
    virtual ~SubgameSolver();
    
    void Solve(void);

    double Time(void) const   { return time; }
    const gList<BehavSolution<T> > &GetSolutions(void) const
      { return solutions; }
};


#endif   // SUBSOLVE_H



