//#
//# FILE: subsolve.h -- Interface to generic subgame solver
//#
//# $Id$
//#

#ifndef SUBSOLVE_H
#define SUBSOLVE_H

#include "efg.h"

template <class T> class SubgameSolver   {
  private:
    Efg<T> efg;
    BehavProfile<T> solution;

    gArray<gArray<Infoset *> *> infosets;

    void FindSubgames(Node *);

  protected:
    virtual void SolveSubgame(const Efg<T> &, BehavProfile<T> &) = 0;

  public:
    SubgameSolver(const Efg<T> &E);
    virtual ~SubgameSolver();
    
    const BehavProfile<T> &Solve(void);
};

#include "seqform.h"

template <class T> class SeqFormBySubgame : public SubgameSolver<T>  {
  private:
    SeqFormParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    SeqFormBySubgame(const Efg<T> &E, const SeqFormParams &);
    virtual ~SeqFormBySubgame();
};

#include "eliap.h"

template <class T> class EFLiapBySubgame : public SubgameSolver<T>  {
  private:
    EFLiapParams<T> params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    EFLiapBySubgame(const Efg<T> &E, const EFLiapParams<T> &);
    virtual ~EFLiapBySubgame();
};

#include "nfg.h"

#include "lemke.h"

template <class T> class LemkeBySubgame : public SubgameSolver<T>  {
  private:
    LemkeParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    LemkeBySubgame(const Efg<T> &E, const LemkeParams &);
    virtual ~LemkeBySubgame();
};

#include "nliap.h"

template <class T> class NFLiapBySubgame : public SubgameSolver<T>  {
  private:
    NFLiapParams<T> params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    NFLiapBySubgame(const Efg<T> &E, const NFLiapParams<T> &);
    virtual ~NFLiapBySubgame();
};

#include "simpdiv.h"

template <class T> class SimpdivBySubgame : public SubgameSolver<T>  {
  private:
    SimpdivParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    SimpdivBySubgame(const Efg<T> &E, const SimpdivParams &);
    virtual ~SimpdivBySubgame();
};

#include "enum.h"

template <class T> class EnumBySubgame : public SubgameSolver<T>  {
  private:
    EnumParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    EnumBySubgame(const Efg<T> &E, const EnumParams &);
    virtual ~EnumBySubgame();
};


#include "purenash.h"

template <class T> class PureNashBySubgame : public SubgameSolver<T>  {
  private:
    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    PureNashBySubgame(const Efg<T> &E);
    virtual ~PureNashBySubgame();
};


#include "csum.h"

template <class T> class ZSumBySubgame : public SubgameSolver<T>  {
  private:
    ZSumParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    ZSumBySubgame(const Efg<T> &E, const ZSumParams &);
    virtual ~ZSumBySubgame();
};



#endif   // SUBSOLVE_H



