//#
//# FILE: csum.h -- Interface to Constant Sum Game Solution Solver
//#
//# $Id$
//#

#ifndef CSUM_H
#define CSUM_H

#include "normal.h"
#include "rational.h"
#include "glist.h"
#include "gstatus.h"
#include "tableau.h"
#include "lpsolve.h"
#include "mixed.h"

class ZSumParams     {
  public:
    int trace, stopAfter;
    gOutput *tracefile;
    
    ZSumParams(void);
};

//
// The general-purpose interface to the solver routines
//

template <class T> class ZSumModule  {
private:
  const NormalForm<T> &NF;
  const ZSumParams &params;
  gMatrix<T> *A;
  gVector<T> *b, *c;
  T minpay;
  long npivots;
  double time;
  BFS_List List;

public:
  ZSumModule(const NormalForm<T> &N, const ZSumParams &p);
  virtual ~ZSumModule();

  bool IsConstSum();
  
  int ZSum(int dup = 0);

  void Make_Abc();
  
  int Add_BFS(const LPSolve<T> &B);
  long NumPivots(void) const;
  double Time(void) const;
  
  void GetSolutions(gList<MixedProfile<T> > &) const;
};

//
// Convenience function for "one-shot" evaluations
//

/*
template <class T> int ZSum(const NormalForm<T> &N, const ZSumParams &p,
			     gList<MixedProfile<T> > &solutions,
			     long &npivots, gRational &time);
*/				   

#endif    // CSUM_H



