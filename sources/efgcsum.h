//
// FILE: efgcsum.h -- Interface to Constant Sum Extensive Form Game Solver 
//
// $Id$
//

#ifndef EFGCSUM_H
#define EFGCSUM_H

#include "efg.h"
#include "glist.h"
#include "gstatus.h"
#include "tableau.h"
#include "lpsolve.h"
#include "behav.h"
#include "behavsol.h"

class CSSeqFormParams     {
  public:
    int trace, stopAfter;
    gOutput *tracefile;
    gStatus &status;
    
    CSSeqFormParams(gStatus &status_ = gstatus);
};

//
// The general-purpose interface to the solver routines
//

template <class T> class CSSeqFormModule  {
private:
  const Efg &EF;
  const CSSeqFormParams &params;
  const EFSupport &support;
  gMatrix<T> *A;
  gVector<T> *b, *c;
  T maxpay, minpay;
  long npivots;
  double time;
  int ns1,ns2,ni1,ni2;
  gList<BFS<T> > List;
  void FillTableau(const Node *, T ,int ,int , int ,int );

public:
  CSSeqFormModule(const Efg &E, const CSSeqFormParams &p, const EFSupport &);
  virtual ~CSSeqFormModule();

  bool IsConstSum();
  
  int CSSeqForm(int dup = 0);

  void Make_Abc();
  
  int Add_BFS(const LPSolve<T> &B);
  long NumPivots(void) const;
  double Time(void) const;
  
  void GetProfile(gDPVector<T> &v, const BFS<T> &sol,
		  const Node *n, int s1,int s2) const;
  void GetSolutions(gList<BehavSolution<T> > &) const;
};


#include "subsolve.h"

template <class T> class CSSeqFormBySubgame : public SubgameSolver<T>  {
  private:
    long npivots;
    CSSeqFormParams params;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution<T> > &);
    EfgAlgType AlgorithmID() const { return EfgAlg_CSSEQFORM; }    

  public:
    CSSeqFormBySubgame(const Efg &E, const EFSupport &,
		       const CSSeqFormParams &, int max = 0);
    virtual ~CSSeqFormBySubgame();

    long NumPivots(void) const  { return npivots; }
};


#endif    // EFGCSUM_H



