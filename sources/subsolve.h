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
    double time;
    Efg<T> efg;
    BehavProfile<T> solution;
    gList<BehavProfile<T> > solutions;

    gArray<gArray<Infoset *> *> infosets;

    void FindSubgames(Node *, BehavProfile<T> &);

  protected:
    virtual void SolveSubgame(const Efg<T> &, BehavProfile<T> &) = 0;

  public:
    SubgameSolver(const Efg<T> &E);
    virtual ~SubgameSolver();
    
    void Solve(void);

    double Time(void) const   { return time; }
    const gList<BehavProfile<T> > GetSolutions(void) const
      { return solutions; }
};

#include "seqform.h"

template <class T> class SeqFormBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    SeqFormParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    SeqFormBySubgame(const Efg<T> &E, const SeqFormParams &);
    virtual ~SeqFormBySubgame();

    int NumPivots(void) const  { return npivots; }
};

#include "eliap.h"

template <class T> class EFLiapBySubgame : public SubgameSolver<T>  {
  private:
    int nevals;
    EFLiapParams params;
    BehavProfile<T> start;
    
    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    EFLiapBySubgame(const Efg<T> &E, const EFLiapParams &,
		    const BehavProfile<T> &);
    virtual ~EFLiapBySubgame();

    int NumEvals(void) const   { return nevals; }
};

#include "nfg.h"

#include "lemke.h"

template <class T> class LemkeBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    LemkeParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    LemkeBySubgame(const Efg<T> &E, const LemkeParams &);
    virtual ~LemkeBySubgame();

    int NumPivots(void) const   { return npivots; }
};

#include "nliap.h"

template <class T> class NFLiapBySubgame : public SubgameSolver<T>  {
  private:
    int nevals;
    NFLiapParams params;
    BehavProfile<T> start;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    NFLiapBySubgame(const Efg<T> &E, const NFLiapParams &,
		    const BehavProfile<T> &);
    virtual ~NFLiapBySubgame();

    int NumEvals(void) const   { return nevals; }
};

#include "simpdiv.h"

template <class T> class SimpdivBySubgame : public SubgameSolver<T>  {
  private:
    int nevals;
    SimpdivParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    SimpdivBySubgame(const Efg<T> &E, const SimpdivParams &);
    virtual ~SimpdivBySubgame();

    int NumEvals(void) const    { return nevals; }
};

#include "enum.h"

template <class T> class EnumBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    EnumParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    EnumBySubgame(const Efg<T> &E, const EnumParams &);
    virtual ~EnumBySubgame();

    int NumPivots(void) const   { return npivots; }
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
    int npivots;
    ZSumParams params;

    void SolveSubgame(const Efg<T> &, BehavProfile<T> &);

  public:
    ZSumBySubgame(const Efg<T> &E, const ZSumParams &);
    virtual ~ZSumBySubgame();

    int NumPivots(void) const   { return npivots; }
};



#endif   // SUBSOLVE_H



