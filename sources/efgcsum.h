//#
//# FILE: efcsum.h -- Interface to Constant Sum Extensive Form Game Solver 
//#
//# $Id$
//#

#ifndef EFCSUM_H
#define EFCSUM_H

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
  const Efg<T> &EF;
  const CSSeqFormParams &params;
  const EFSupport &support;
  gMatrix<T> *A;
  gVector<T> *b, *c;
  T maxpay;
  long npivots;
  double time;
  int ns1,ns2,ni1,ni2;
  BFS_List List;
  void FillTableau(const Node *, T ,int ,int , int ,int );

public:
  CSSeqFormModule(const Efg<T> &E, const CSSeqFormParams &p, const EFSupport &);
  virtual ~CSSeqFormModule();

  bool IsConstSum();
  
  int CSSeqForm(int dup = 0);

  void Make_Abc();
  
//  int Add_BFS(const LPSolve<T> &B);
  long NumPivots(void) const;
  double Time(void) const;
  
//  void GetProfile(gDPVector<T> &v, const gVector<T> &sol,
//		  const Node *n, int s1,int s2);
  void GetSolutions(gList<BehavSolution<T> > &) const;
};

//
// Convenience function for "one-shot" evaluations
//

/*
template <class T> int CSSeqForm(const Efg<T> &E, const CSSeqFormParams &p,
			     gList<BehavProfile<T> > &solutions,
			     long &npivots, gRational &time);
*/				   

#endif    // EFCSUM_H



