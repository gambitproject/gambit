//#
//# FILE: subsolve.h -- Interface to generic subgame solver
//#
//# $Id$
//#

#ifndef SUBSOLVE_H
#define SUBSOLVE_H

#include "efg.h"
#include "nfg.h"
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
    virtual void SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &) = 0;

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

#include "seqform.h"

template <class T> class SeqFormBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    SeqFormParams params;

    void SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_SEQFORMSUB; }    

  public:
    SeqFormBySubgame(const Efg<T> &E, const SeqFormParams &, int max = 0);
    virtual ~SeqFormBySubgame();

    int NumPivots(void) const  { return npivots; }
};

#include "eliap.h"

class EFLiapBySubgame : public SubgameSolver<double>  {
  private:
    int nevals;
    EFLiapParams params;
    BehavProfile<double> start;
    
    void SolveSubgame(const Efg<double> &, gList<BehavSolution<double> > &);
    int AlgorithmID() const { return id_ELIAPSUB; }    

  public:
    EFLiapBySubgame(const Efg<double> &E, const EFLiapParams &,
		    const BehavProfile<double> &, int max = 0);
    virtual ~EFLiapBySubgame();

    int NumEvals(void) const   { return nevals; }
};


#include "lemke.h"

template <class T> class LemkeBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    LemkeParams params;

    void SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_LEMKESUB; }    

  public:
    LemkeBySubgame(const Efg<T> &E, const LemkeParams &, int max = 0);
    virtual ~LemkeBySubgame();

    int NumPivots(void) const   { return npivots; }
};

#include "nliap.h"

class NFLiapBySubgame : public SubgameSolver<double>  {
  private:
    int nevals;
    NFLiapParams params;
    BehavProfile<double> start;

    void SolveSubgame(const Efg<double> &,
		      gList<BehavSolution<double> > &);
    int AlgorithmID() const { return id_NLIAPSUB; }    

  public:
    NFLiapBySubgame(const Efg<double> &E, const NFLiapParams &,
		    const BehavProfile<double> &, int max = 0);
    virtual ~NFLiapBySubgame();

    int NumEvals(void) const   { return nevals; }
};

#include "simpdiv.h"

template <class T> class SimpdivBySubgame : public SubgameSolver<T>  {
  private:
    int nevals;
    SimpdivParams params;

    void SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_SIMPDIVSUB; }    

  public:
    SimpdivBySubgame(const Efg<T> &E, const SimpdivParams &, int max = 0);
    virtual ~SimpdivBySubgame();

    int NumEvals(void) const    { return nevals; }
};

#include "enum.h"

template <class T> class EnumBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    EnumParams params;

    void SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_ENUMSUB; }    

  public:
    EnumBySubgame(const Efg<T> &E, const EnumParams &, int max = 0);
    virtual ~EnumBySubgame();

    int NumPivots(void) const   { return npivots; }
};


#include "purenash.h"

template <class T> class PureNashBySubgame : public SubgameSolver<T>  {
  private:
    void SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_PURENASHSUB; }    

  public:
    PureNashBySubgame(const Efg<T> &E, int max = 0);
    virtual ~PureNashBySubgame();
};


#include "csum.h"

template <class T> class ZSumBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    ZSumParams params;

    void SolveSubgame(const Efg<T> &, gList<BehavSolution<T> > &);
    int AlgorithmID() const { return id_ZSUMSUB; }    

  public:
    ZSumBySubgame(const Efg<T> &E, const ZSumParams &, int max = 0);
    virtual ~ZSumBySubgame();

    int NumPivots(void) const   { return npivots; }
};



#endif   // SUBSOLVE_H



