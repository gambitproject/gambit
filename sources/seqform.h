//
// FILE: seqform.h -- Interface to Sequence Form solution module
//
// $Id$
//

#ifndef SEQFORM_H
#define SEQFORM_H

#include "efg.h"
#include "glist.h"
#include "gmatrix.h"
#include "lemketab.h"
#include "gstatus.h"
#include "behavsol.h"

class SeqFormParams     {
  public:
    int trace, stopAfter, maxdepth;
    gOutput *tracefile;
    gStatus &status;
    
    SeqFormParams(gStatus &status_ = gstatus);
};

//
// The general-purpose interface to the solver routines
//

template <class T> class SeqFormModule  {
private:
  const Efg &EF;
  const EFSupport &support;
  const SeqFormParams &params;
  gMatrix<T> *A;
  gVector<T> *b;
  LTableau<T> *tab;
  int ns1,ns2,ni1,ni2;
  T maxpay,eps;
  long npivots;
  double time;
  gList<BFS<T> > List;
  gList<BehavSolution> solutions;

public:
  SeqFormModule(const Efg &E, const SeqFormParams &p,
		const EFSupport &);
  virtual ~SeqFormModule();
  
  int Lemke(void);
  int All_Lemke(int dup, LTableau<T> &B, long &np, int depth);
  
  int Add_BFS(const LTableau<T> &tab);
  long NumPivots(void) const;
  double Time(void) const;
  
  void FillTableau(const Node *n,T prob,int s1,int s2,int i1,int i2);
  void GetProfile(const LTableau<T> &tab, gDPVector<T> &, const gVector<T> &, 
		  const Node *n, int,int);

  void AddSolutions(void);
  const gList<BehavSolution> &GetSolutions() const;
};


#include "subsolve.h"

template <class T> class SeqFormBySubgame : public SubgameSolver<T>  {
  private:
    int npivots;
    SeqFormParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_SEQFORMSUB; }    

  public:
    SeqFormBySubgame(const Efg &E, const EFSupport &,
                     const SeqFormParams &, int max = 0);
    virtual ~SeqFormBySubgame();

    int NumPivots(void) const  { return npivots; }
};


#endif    // SEQFORM_H



