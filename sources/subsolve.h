//#
//# FILE: subgame.h -- Interface to generic subgame solver
//#
//# $Id$
//#

#ifndef SUBGAME_H
#define SUBGAME_H

#include "efg.h"

template <class T> class SubgameSolver   {
  private:
    Efg<T> efg;
    BehavProfile<T> solution;

    gArray<gArray<Infoset *> *> infosets;

    void FindSubgames(Node *);

  protected:
    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    SubgameSolver(const Efg<T> &E);
    ~SubgameSolver();
    
    const BehavProfile<T> &Solve(void);
};

#endif   // SUBGAME_H

